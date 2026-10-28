/*
 * File: tty.c
 * File Created: 28 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/ttyt.h"
#include "drivers/tty/tty.h"
#include "drivers/tty/termbits.h"

#include "drivers/base/class.h"

#include "dev/device.h"

#include "ringbuf/ringbuf.h"

#include "init.h"

#include "kerrno.h"

#include "memory.h"

#define INIT_C_CC {          \
    [VINTR] = 'C' & 0x1F,    \
    [VQUIT] = '\\' - 0x40,   \
    [VERASE] = '\177',       \
    [VKILL] = 'U' - 0x40,    \
    [VEOF] = 'D' - 0x40,     \
    [VTIME] = 0,             \
    [VMIN] = 1,              \
    [VSWTC] = 0,             \
    [VSTART] = 'Q' - 0x40,   \
    [VSTOP] = 'S' - 0x40,    \
    [VSUSP] = 'Z' - 0x40,    \
    [VEOL] = 0,              \
    [VREPRINT] = 'R' - 0x40, \
    [VDISCARD] = 'O' - 0x40, \
    [VWERASE] = 'W' - 0x40,  \
    [VLNEXT] = 'V' - 0x40,   \
    [VEOL2] = 0}

termios_t tty_std_termios = {
    /* for the benefit of tty drivers  */
    .c_iflag = ICRNL | IXON,
    .c_oflag = OPOST | ONLCR,
    .c_cflag = B38400 | CS8 | CREAD | HUPCL,
    .c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
        ECHOCTL | ECHOKE | IEXTEN,
    .c_cc = INIT_C_CC,
    .c_ispeed = 38400,
    .c_ospeed = 38400,
    /* .c_line = N_TTY, */
};

status_t tty_added(device_t *dev)
{
    return KERRNO_SUCCESSES;
}

static dev_class_t tty_class = {
    .name = "tty",
    .type = DEVICE_TTY,
    .device_added = tty_added,
};

tty_struct_t *active_tty;

status_t tty_init()
{
    class_register(&tty_class);
    active_tty = kzmalloc(sizeof(tty_struct_t));

    memcpy(&active_tty->termios, &tty_std_termios, sizeof(tty_std_termios));

    // ringbuf_init(sizeof(wchar_t), 128, &active_tty->char_buffer);

    active_tty->tty_dev.init_name = "tty";

    active_tty->tty_dev.class = &tty_class;

    dev_set_no_driver(&active_tty->tty_dev);
    dev_set_same_name(&active_tty->tty_dev);
    device_register(&active_tty->tty_dev);
    return KERRNO_SUCCESSES;
}

DEVICE_INITCALL(tty_init);

