/*
 * File: tty_ioctl.c
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty.h"
#include "tty_ioctl.h"
#include "tty_types.h"
#include "tty_flags.h"

#include "kernel/memory.h"

#include <util/arrays.h>

#include "errno/errno.h"

int tty_ioctl_mode(tty_struct_t *tty, uint32_t cmd, void *arg)
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
            // copy window size → userspace
            // memcpy(arg, &tty->winsize, sizeof(winsize_t));
            return RETURN_GOOD;

        case TIOCSWINSZ :
            // memcpy(&tty->winsize, arg, sizeof(winsize_t));
            // optionally send SIGWINCH to fg process group
            // tty_signal(tty, SIGWINCH);
            return RETURN_GOOD;

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
            return -ENOTTY; // not a tty ioctl
    }
}
