/*
 * File: tty.c
 * File Created: 15 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty.h"
#include "tty_types.h"
#include "tty_config.h"
#include "tty_flags.h"

#include "kernel/memory.h"
#include "kernel/string.h"
#include "device/device.h"

#include "drivers/IO/keyboard/Keyboard.h"
#include "drivers/IO/keyboard/GKI.h"

#include "errno/errno.h"
#include <signals.h>

#define MODULE "TTY"

int tty_count = 0;

size_t tty_read_dev(void *buf, off_t offset, size_t count, device_t *dev);
size_t tty_write_dev(void *buf, off_t offset, size_t count, device_t *dev);
int tty_ioctl_dev(uint32_t cmd, void *arg, device_t *dev);
void tty_destroy_dev(device_t *dev);

// --- lifecycle ---

// allocate and initialise a tty, hook it to a gki ring buffer
tty_struct_t *tty_create(device_t *input_dev, device_t *output_dev)
{
    tty_struct_t *new = malloc(sizeof(tty_struct_t));
    memset(new, 0, sizeof(tty_struct_t));
    new->input_dev = input_dev;
    new->output_dev = output_dev;
    tty_termios_init(new);

    if (input_dev)
    {
        if (input_dev->tty_ops)
        {
            input_dev->tty_ops(input_dev, &new->in_ops);
        }
    }
    if (output_dev)
    {
        if (output_dev->tty_ops)
        {
            output_dev->tty_ops(input_dev, &new->out_ops);
        }
    }

    device_t *tty_device = malloc(sizeof(device_t));
    memset(tty_device, 0, sizeof(device_t));
    tty_device->class_name = "tty";
    tty_device->type = DEVICE_TTY;
    tty_device->flags = DEVICE_FLAG_RW | DEVICE_FLAG_BLOCKDEV | DEVICE_FLAG_VIRTUAL;
    tty_device->read = tty_read_dev;
    tty_device->write = tty_write_dev;
    tty_device->ioctl = tty_ioctl_dev;
    tty_device->destroy = tty_destroy_dev;
    tty_device->priv = new;
    device_register(tty_device);
    new->id = (int)tty_device->device_id;

    return new;
}

// tear down
void tty_destroy(tty_struct_t *tty)
{
}

// --- signal handling ---

// send a signal to the foreground process group
void tty_signal(tty_struct_t *tty, int signo)
{
}

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
    if (FLAG_IS_SET(tty->termios.c_lflag, ICANON) && c == '\n')
    {
        tty->line_buf.has_line = true;
    }
}

void tty_process_event(tty_struct_t *tty, gki_event_t *ev)
{
    ENTER_FUNC(MODULE, "%p, %p", tty, ev);
    // drop key release events
    // (assuming GKI already filters these, but just in case)
    log_debug(MODULE, "%u, %c, %u", ev->key, ev->key_char, ev->modifier);

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

// --- syscall interface ---

// called from your read() syscall handler
// blocks until a complete line is available (in canonical mode)
ssize_t tty_read(tty_struct_t *tty, uint8_t *buf, size_t n)
{
    ENTER_FUNC(MODULE, "%p, %p, %u", tty, buf, n);
    log_debug(MODULE, "c_lflag = %x", tty->termios.c_lflag);
    if (FLAG_IS_SET(tty->termios.c_lflag, ICANON))
    {
        log_debug(MODULE, "has_line = %s", tty->line_buf.has_line BOOL_TO_STRING);
        while (!tty->line_buf.has_line)
        {
            tty_drain(tty);
        }

        return tty_linebuf_read(tty, buf, n);
    }
    return 0;
}

// called from your write() syscall handler
// writes buf to the output side, handling \n -> \r\n if ONLCR set
ssize_t tty_write(tty_struct_t *tty, const uint8_t *buf, size_t n)
{
    ENTER_FUNC(MODULE, "%p, %p, %u", tty, buf, n);
    for (size_t i = 0; i < n; i++)
    {
        uint8_t c = buf[i];
        if (c == '\n' && FLAG_IS_SET(tty->termios.c_oflag, ONLCR))
        {
            tty->out_ops.write_char(tty->output_dev, '\r');
        }
        tty->out_ops.write_char(tty->output_dev, c);
    }
    return n;
}

// called from your ioctl() syscall handler
int tty_ioctl(tty_struct_t *tty, uint32_t cmd, void *arg)
{

    return RETURN_FAILED;
}

size_t tty_read_dev(void *buf, off_t offset, size_t count, device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    return tty_read(tty, buf, count);
}

size_t tty_write_dev(void *buf, off_t offset, size_t count, device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    return tty_write(tty, buf, count);
}

int tty_ioctl_dev(uint32_t cmd, void *arg, device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    return tty_ioctl(tty, cmd, arg);
}

void tty_destroy_dev(device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    tty_destroy(tty);
}

