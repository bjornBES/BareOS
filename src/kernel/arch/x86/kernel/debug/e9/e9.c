/*
 * File: e9.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "e9.h"

#include "kernel/io.h"
#include <defs.h>

INLINE void e9_putc(char c)
{
    outb(0xE9, c);
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

void debug_write_char(char c)
{
    e9_putc(c);
}