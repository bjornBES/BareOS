/*
 * File: tty_io.c
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty.h"
#include "tty_types.h"
#include "tty_flags.h"
#include "tty_ioctl.h"

#include "drivers/IO/keyboard/Keyboard.h"
#include "drivers/IO/keyboard/GKI.h"

#include "task/threading/thread.h"
#include "task/threading/scheduling/scheduler.h"

#include "errno/errno.h"

#include "kernel/memory.h"

#define MODULE "TTY-IO"

// --- echo ---

// write a char to the output side (respects termios ECHO flag)
void tty_echo(tty_struct_t *tty, char c)
{
    device_write(tty->output_dev, &c, 0, 1);
}

// echo a backspace (writes "\b \b" to erase the character visually)
void tty_echo_erase(tty_struct_t *tty)
{
    device_write(tty->output_dev, "\b \b", 0, 3);
}

// --- line buffer ---

// push a byte into the line buffer
void tty_linebuf_push(tty_struct_t *tty, uint8_t byte)
{
    tty_line_buf_t *lb = &tty->line_buf;
    if (lb->head >= TTY_BUF_SIZE)
    {
        return; // buffer full, drop
    }

    lb->buf[lb->head++] = byte;
}

// erase the last byte from the line buffer
void tty_linebuf_erase(tty_struct_t *tty)
{
    tty_line_buf_t *lb = &tty->line_buf;
    if (lb->head >= TTY_BUF_SIZE)
    {
        return; // buffer full, drop
    }
    lb->head--;
    if (FLAG_IS_SET(tty->termios.c_lflag, ECHOE))
    {
        tty_echo_erase(tty);
    }
}

// copy completed line out to userspace buffer, returns bytes copied
size_t tty_linebuf_read(tty_struct_t *tty, uint8_t *dst, size_t n)
{
    tty_line_buf_t *lb = &tty->line_buf;
    size_t avail = lb->head - lb->tail;
    size_t count = avail < n ? avail : n;
    memcpy(dst, lb->buf + lb->tail, count);
    lb->tail += count;
    // if fully consumed reset buffer
    if (lb->tail >= lb->head)
    {
        lb->head = 0;
        lb->tail = 0;
        lb->has_line = false;
    }
    return count;
}

// --- core input path ---

void tty_process_byte(tty_struct_t *tty, uint8_t c)
{
    // map \r → \n if ICRNL set
    if (c == '\r' && FLAG_IS_SET(tty->termios.c_iflag, ICRNL))
    {
        c = '\n';
    }

    // echo
    if (FLAG_IS_SET(tty->termios.c_lflag, ECHO))
    {
        tty_echo(tty, c);
    }

    // push into line buffer
    tty_linebuf_push(tty, c);

    // if canonical mode and we got a newline, mark line complete
    if (FLAG_IS_SET(tty->termios.c_lflag, ICANON) && (c == '\n' || c == '\r'))
    {
        tty->line_buf.has_line = true;
    }
}

void tty_process_event(tty_struct_t *tty, gki_event_t *ev)
{
    ENTER_FUNC(MODULE, "%p, %p", tty, ev);
    // drop key release events
    // (assuming GKI already filters these, but just in case)
    log_debug(MODULE, "key = %u, '%c', mod = 0x%x", ev->key, ev->key_char, ev->modifier);

    // handle ctrl combos
    if (FLAG_IS_SET(ev->modifier, MOD_CTRL) && ev->key_char != 0)
    {
        uint8_t ctrl = ev->key_char & 0x1F;
        if (FLAG_IS_SET(tty->termios.c_lflag, ISIG))
        {
            if (ctrl == tty->termios.c_cc[VINTR])
            {
                // Ctrl+C
                tty_signal(tty, SIGINT);
                return;
            }
            if (ctrl == tty->termios.c_cc[VSUSP])
            {
                // Ctrl+Z
                tty_signal(tty, SIGTSTP);
                return;
            }
        }
        if (ctrl == tty->termios.c_cc[VEOF])
        {
            // Ctrl+D
            tty->line_buf.has_line = true; // wake reader with 0 bytes = EOF
            return;
        }
    }

    if (ev->key == KEY_BACKSPACE || ev->key_char == tty->termios.c_cc[VERASE])
    {
        tty_linebuf_erase(tty);
        return;
    }

    // non printable keys with no key_char (arrows, fkeys etc.)
    if (ev->key_char == 0)
    {
        // TODO: emit ANSI escape sequences for arrows etc.
        return;
    }

    // normal printable character
    tty_process_byte(tty, (uint8_t)ev->key_char);
}

void tty_drain(tty_struct_t *tty)
{
    // ENTER_FUNC(MODULE, "%p", tty);
    if (!tty->input_dev)
    {
        return;
    }

    gki_event_t ev;
    ringbuf_index buf = tty->in_ops.get_input_buf(tty->input_dev);
    while (ringbuf_read(buf, &ev, 1) != 0)
    {
        tty_process_event(tty, &ev);
    }
}

uintptr_t tty_io_thread(void *arg)
{
    tty_struct_t *tty = arg;
    while (1)
    {
        // sleep until GKI ring buffer has data
        // wait_event(tty->gki_wait, !ringbuf_empty(tty->gki_buf));

        // drain and process
        tty_drain(tty);

        // if line is complete wake any blocked readers
        if (tty->line_buf.has_line)
        {
            sched_wake_one(&tty->read_wait);
            return 0;
        }
    }
}

// blocks until a complete line is available (in canonical mode)
ssize_t tty_read(tty_struct_t *tty, uint8_t *buf, size_t n)
{
    ENTER_FUNC(MODULE, "%p, %p, %u", tty, buf, n);
    irq_arch_enable();

    thread_t *t = thread_create_kernel(tty_io_thread, tty);
    thread_set_priority(t, PRIORITY_MIDDLE_SHORT);
    sched_add(t);
    sched_block(&tty->read_wait);
    
    // log_debug(MODULE, "c_lflag = %x", tty->termios.c_lflag);
    if (FLAG_IS_SET(tty->termios.c_lflag, ICANON))
    {
        // log_debug(MODULE, "has_line = %s", tty->line_buf.has_line BOOL_TO_STRING);
        while (!tty->line_buf.has_line)
        {
            tty_drain(tty);
        }

        return tty_linebuf_read(tty, buf, n);
    }
    return 0;
}

void tty_io_drain_output(tty_struct_t *tty)
{
}

void tty_io_linebuf_flush(tty_struct_t *tty)
{
}

int tty_io_gwinsz(tty_struct_t *tty, winsize_t *arg)
{
    int state = copy_to_user(arg, &tty->winsize, sizeof(winsize_t));
    if (state != RETURN_GOOD)
    {
        ERRNO_RETURN(EINVAL, "argp is invalid", 0);
    }
    return RETURN_GOOD;
}

int tty_io_swinsz(tty_struct_t *tty, winsize_t *arg)
{
    int state = copy_from_user(arg, &tty->winsize, sizeof(winsize_t));
    if (state != RETURN_GOOD)
    {
        ERRNO_RETURN(EINVAL, "argp is invalid", 0);
    }
    return RETURN_GOOD;
}

int tty_ioctl(tty_struct_t *tty, uint32_t cmd, void *arg)
{
    switch (cmd)
    {
        case TCSETSW :
            // drain output first, then set
            tty_io_drain_output(tty);
            goto TCSETS_;

        case TCSETSF :
            // flush input, then set
            tty_io_linebuf_flush(tty);
            goto TCSETS_;

        case TIOCGWINSZ :
            return tty_io_gwinsz(tty, arg);

        case TIOCSWINSZ :
            return tty_io_swinsz(tty, arg);

        case TCGETS :
            // copy kernel termios → userspace
            copy_to_user(arg, &tty->termios, sizeof(termios_t));
            return RETURN_GOOD;

        case TCSETS :
TCSETS_:
            // copy userspace termios → kernel
            copy_from_user(&tty->termios, arg, sizeof(termios_t));
            return RETURN_GOOD;

        default :
            ERRNO_RETURN(ENOTTY, "The specified operation does not apply to the kind of object that the file descriptor fd references.", 0);
    }
}
