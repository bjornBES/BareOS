/*
 * File: tty_io.c
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty_types.h"
#include "tty_flags.h"
#include "tty_ioctl.h"

#include "errno/errno.h"

#include "kernel/memory.h"

#define MODULE "TTY-IO"

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
