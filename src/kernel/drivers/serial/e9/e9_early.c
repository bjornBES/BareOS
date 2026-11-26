/*
 * File: e9_early.c
 * File Created: 22 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "console/earlycon.h"

#include "asm/io_arch.h"

#include "kerrno.h"

static const char *const color_reset = "\033[0m";
static const char *const log_severity_colors[] =
    {
        [LVL_FUNC_ENTER] = "\033[2;37m",
        [LVL_DEBUG] = "\033[2;37m",
        [LVL_INFO] = "\033[37m",
        [LVL_SCHED] = "\033[2;37m",
        [LVL_NOTICE] = "\033[1;37m",
        [LVL_WARNING] = "\033[1;33m",
        [LVL_ERROR] = "\033[1;31m",
        [LVL_CRITICAL] = "\033[1;37;31m",
        [LVL_ALERT] = "\033[1;91m",
        [LVL_EMERG] = "\033[1;43;91m",
};

static status_t early_e9_write(console_t *console, console_message_t *console_msg, const char *s, size_t count)
{
    char *start = (char *)log_severity_colors[console_msg->log_level];
    while (*start)
    {
        outb(0xE9, *start);
        start++;
    }
    for (size_t i = 0; i < count; i++, s++)
    {
        if (*s == '\n')
        {
            start = (char *)color_reset;
            while (*start)
            {
                outb(0xE9, *start);
                start++;
            }
        }
        outb(0xE9, *s);
    }
    start = (char *)color_reset;
    while (*start)
    {
        outb(0xE9, *start);
        start++;
    }
    return KERRNO_SUCCESS;
}

int __init early_e9_setup(earlycon_device_t *device, const char *options)
{
    if (inb(0xE9) != 0xE9)
    {
        return KERRNO_DEV_NOT_FOUND;
    }

    device->con->write = early_e9_write;

    device->con->console_loglevel = CON_WRITE_LEVEL_ENTER_FUNC;

    return KERRNO_SUCCESS;
}

EARLYCON_DECLARE(e9, early_e9_setup);
