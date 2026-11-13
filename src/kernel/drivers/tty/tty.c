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

#include "vfs/filesystems/devfs/devfs.h"

#include "drivers/tty/vt/vt.h"

#include "drivers/base/core.h"

#include "drivers/base/class.h"

#include "dev/device.h"

#include "ringbuf/ringbuf.h"

#include "lists/list.h"

#include "init.h"

#include "kerrno.h"

#include "memory.h"

#define MODULE "tty"

#define INIT_C_CC                \
    {                            \
        [VINTR] = 'C' - 0x40,    \
        [VQUIT] = '\\' - 0x40,   \
        [VERASE] = '\b',       \
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
        [VEOL2] = 0,             \
    }

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

device_t *console_dev;

list_t tty_drivers;

status_t tty_dev_open(device_t *dev, int flags, vfs_node_t *out_file)
{
    ENTER_FUNC("%p, 0x%x, %p", dev, flags, out_file);
    tty_dev_t *tty_dev = to_tty_dev(dev);
    trace_debug(MODULE, "tty_dev @ %p, tty_dev->driver = %p, tty_dev->driver_data = %p, tty_dev->tty = %p", tty_dev, tty_dev->driver, tty_dev->driver_data, tty_dev->tty);
    return tty_do_open(tty_dev->tty, out_file);
}

status_t tty_added(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    dev->ops.open = tty_dev_open;
    return KERRNO_SUCCESSES;
}

status_t tty_removed(device_t *dev)
{
    return KERRNO_SUCCESSES;
}

tty_struct_t *active_tty;

status_t tty_devfs_dev_open(device_t *dev, int flags, vfs_node_t *out_file)
{
    ENTER_FUNC("%p, 0x%x, %p", dev, flags, out_file);
    return tty_do_open(active_tty, out_file);
}

status_t tty_devfs_dev_read(device_t *dev, void *buf, size_t len, off_t off, ssize_t *size_out)
{
    ENTER_FUNC("%p, %p, 0x%x, 0x%x, %p", dev, buf, len, off, size_out);
    FUNC_NOT_IMPLEMENTED();
    // tty_write(active_tty, buf, len, size_out);
    return KERRNO_SUCCESSES;
}

status_t tty_devfs_dev_write(device_t *dev, const void *buf, size_t len, off_t off, ssize_t *size_out)
{
    ENTER_FUNC("%p, %p, 0x%x, 0x%x, %p", dev, buf, len, off, size_out);
    size_t length = len;
    uint8_t *buffer = (void *)buf;
    while (length || *buffer)
    {
        ssize_t size = 0;
        tty_write(active_tty, buffer, 1, &size);
        length--;
        buffer++;
    }

    // tty_write(active_tty, (void *)buf, len, size_out);
    return KERRNO_SUCCESSES;
}

const dev_class_t tty_class = {
    .name = "tty",
    .type = DEVICE_TTY,
    .device_added = tty_added,
    .device_removed = tty_removed,
};

status_t tty_init()
{
    list_initialize(&tty_drivers);
    class_register(&tty_class);
    // active_tty = kzmalloc(sizeof(tty_struct_t));
    // memcpy(&active_tty->termios, &tty_std_termios, sizeof(tty_std_termios));

    tty_dev_t *tty_dev = core_device_create_type(&tty_class, NULL, tty_dev_t, NULL, "tty");
    tty_dev->dev.ops.open = tty_devfs_dev_open;
    tty_dev->dev.ops.read = tty_devfs_dev_read;
    tty_dev->dev.ops.write = tty_devfs_dev_write;
    devfs_add(&tty_dev->dev);
    
    console_dev = core_device_create(&tty_class, NULL, NULL, "console");
    devfs_add(console_dev);

    // ringbuf_init(sizeof(wchar_t), 128, &active_tty->char_buffer);

#ifdef CONFIG_USE_VT
    return vty_init();
#endif
    return KERRNO_SUCCESSES;
}

DEVICE_INITCALL(tty_init);

