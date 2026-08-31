/*
 * File: debug.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "debug/debug.h"
#include <printf_driver/printf.h>
#include "asm/debug_arch.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
// #include "kernel/irq.h"

static const char *const log_severity_colors[] =
    {
        [LVL_DEBUG] = "\033[2;37m",
        [LVL_INFO] = "\033[37m",
        [LVL_WARN] = "\033[1;33m",
        [LVL_ERROR] = "\033[1;31m",
        [LVL_CRITICAL] = "\033[1;37;41m",
};

static const char *const g_ColorReset = "\033[0m";

// spinlock_t debug_logs = {0};

void logfl(const char *module, DebugLevel level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    logfl_args(module, level, fmt, args);

    va_end(args);
}

void logfl_args(const char *module, DebugLevel level, const char *fmt, va_list args)
{
    if (level < MIN_LOG_LEVEL)
    {
        return;
    }

    // spinlock_acquire_irq(&debug_logs);
    char log[255];
    int count;
    memset(log, 0, 255);
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s] ", log_severity_colors[level], module);
        // set color depending on level
        // write module
        debug_write_line(log, count);
    }
    else
    {
        count = sprintf(log, "%s", log_severity_colors[level]);
        // set color depending on level
        debug_write_line(log, count);
    }
    memset(log, 0, 255);
    count = vsprintf(log, fmt, args); // write text
    debug_write_line(log, count);

    count = sprintf(log, "%s\n", g_ColorReset); // write text
    debug_write_line(log, count);
    // spinlock_release_irq(&debug_logs);
}

void logfl_int(const char *module, DebugLevel level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    logfl_int_args(module, level, fmt, args);

    va_end(args);
}

void logfl_int_args(const char *module, DebugLevel level, const char *fmt, va_list args)
{
    if (level < MIN_LOG_LEVEL)
    {
        return;
    }

    // spinlock_acquire_irq(&debug_logs);
    char log[255];
    int count;
    memset(log, 0, 255);
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s] ", log_severity_colors[level], module);
        // set color depending on level
        // write module
        debug_write_line(log, count);
    }
    else
    {
        count = sprintf(log, "%s", log_severity_colors[level]);
        // set color depending on level
        debug_write_line(log, count);
    }
    memset(log, 0, 255);
    count = vsprintf(log, fmt, args); // write text
    debug_write_line(log, count);

    count = sprintf(log, "%s\n", g_ColorReset); // write text
    debug_write_line(log, count);
    // spinlock_release_irq(&debug_logs);
}

void logf(const char *module, DebugLevel level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    logf_args(module, level, fmt, args);

    va_end(args);
}

void logf_args(const char *module, DebugLevel level, const char *fmt, va_list args)
{
    if (level < MIN_LOG_LEVEL)
    {
        return;
    }

    // spinlock_acquire_irq(&debug_logs);

    char log[255];
    int count;
    memset(log, 0, 255);
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s]", log_severity_colors[level], module);
        // set color depending on level
        // write module
        debug_write_line(log, count);
    }
    else
    {
        count = sprintf(log, "%s", log_severity_colors[level]);
        // set color depending on level
        debug_write_line(log, count);
    }
    memset(log, 0, 255);
    count = vsprintf(log, fmt, args); // write text
    debug_write_line(log, count);

    count = sprintf(log, "%s", g_ColorReset); // write text
    debug_write_line(log, count);
    // spinlock_release_irq(&debug_logs);
}

void debug_enter_func(const char *module, const char *function, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    // spinlock_acquire_irq(&debug_logs);

    char log[255];
    int count;
    memset(log, 0, 255);
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s] Enter %s(", log_severity_colors[LVL_DEBUG], module, function);
        debug_write_line(log, count);
    }
    else
    {
        count = sprintf(log, "%sEnter %s(", log_severity_colors[LVL_DEBUG], function);
        debug_write_line(log, count);
    }
    memset(log, 0, 255);
    count = vsprintf(log, fmt, args); // write text
    debug_write_line(log, count);

    count = sprintf(log, ")%s\n", g_ColorReset); // write text
    debug_write_line(log, count);
    va_end(args);
    // spinlock_release_irq(&debug_logs);
}

