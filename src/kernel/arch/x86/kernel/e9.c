/*
 * File: e9.c
 * File Created: 16 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "debug.h"
#include "drivers/IO/tty/tty_types.h"

void e9_write_char(device_t *dev, char c)
{
    e9_putc(c);
}

void debug_write_char(char c)
{
    e9_putc(c);
}

ssize_t e9_write_data(void *buf, off_t offset, size_t count, device_t *dev)
{
    uint8_t *buffer = (uint8_t *)buf;
    e9_putc(*buffer);
    return 1;
}

size_t debug_write_line(char *buf, size_t count)
{
    uint8_t *buffer = (uint8_t *)buf;
    for (size_t i = 0; i < count; i++)
    {
        e9_putc(buffer[i]);
    }
    return count;
}

void e9_get_ops(device_t *dev, tty_dev_ops_t *ops)
{
    ops->write_char = e9_write_char;
}

void e9_init()
{
    device_t *e9_dev = device_create(DEVICE_CHAR, DEVICE_FLAG_WRITABLE);
    e9_dev->type = DEVICE_CHAR;
    e9_dev->write = e9_write_data;
    e9_dev->tty_ops = e9_get_ops;
    e9_dev->class_name = "debug";
    device_register(e9_dev);
}