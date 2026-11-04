/*
 * File: n_tty.c
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/tty.h"
#include "drivers/tty/tty_ldisct.h"

const tty_ldisc_ops_t n_tty_ops = {
    .num = N_TTY,
    .name = "n_tty",
    // .open = n_tty_open,
    // .close = n_tty_close,
    // .flush_buffer = n_tty_flush_buffer,
    // .read = n_tty_read,
    // .write = n_tty_write,
    // .set_termios = n_tty_set_termios,
    // .poll = n_tty_poll,
    // .receive_buf = n_tty_receive_buf,
    // .write_wakeup = n_tty_write_wakeup,
    // .lookahead_buf = n_tty_lookahead_flow_ctrl,
};

status_t n_tty_init()
{
	return tty_register_ldisc(&n_tty_ops);
}
