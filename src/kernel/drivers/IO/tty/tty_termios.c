/*
 * File: tty_termios.c
 * File Created: 11 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty_types.h"
#include "tty_flags.h"

#include "kernel/memory.h"

#define MODULE "tty-termios"

int tty_termios_set(tty_struct_t *tty, termios_t *term)
{
    ENTER_FUNC(MODULE, "%p, %p", tty, term);
    tty->termios.c_cflag = term->c_cflag;
    tty->termios.c_iflag = term->c_iflag;
    tty->termios.c_lflag = term->c_lflag;
    tty->termios.c_oflag = term->c_oflag;
    return RETURN_GOOD;
}

void tty_termios_init(tty_struct_t *tty)
{

}
void tty_termios_save(tty_struct_t *tty)
{

}