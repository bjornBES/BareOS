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
        [DEBUG_LVL_DEBUG] = "\033[2;37m",
        [DEBUG_LVL_INFO] = "\033[37m",
        [DEBUG_LVL_WARN] = "\033[1;33m",
        [DEBUG_LVL_ERROR] = "\033[1;31m",
        [DEBUG_LVL_CRITICAL] = "\033[1;37;41m",
};

static const char *const g_ColorReset = "\033[0m";

spinlock_t debug_logs = {0};

void logfl(const char *module, debug_level_t level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    logfl_args(module, level, fmt, args);

    va_end(args);
}

void logfl_args(const char *module, debug_level_t level, const char *fmt, va_list args)
{
    char log[255];
    char message[255];
    int count;
    memset(log, 0, sizeof(log));
    memset(message, 0, sizeof(message));
    count = vsprintf(message, fmt, args); // write text
    message[count] = '\0';
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s] %s%s\n", log_severity_colors[level], module, message, g_ColorReset);
        // set color depending on level
        // write module
    }
    else
    {
        count = sprintf(log, "%s %s%s\n", log_severity_colors[level], message, g_ColorReset);
        // set color depending on level
    }
    log[count] = '\0';

    spinlock_acquire(&debug_logs);
    debug_write_line(log, count);
    spinlock_release(&debug_logs);
}

void logf(const char *module, debug_level_t level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    logf_args(module, level, fmt, args);

    va_end(args);
}

void logf_args(const char *module, debug_level_t level, const char *fmt, va_list args)
{
    char log[255];
    char message[255];
    int count;
    memset(log, 0, sizeof(log));
    memset(message, 0, sizeof(message));
    count = vsprintf(message, fmt, args); // write text
    message[count] = '\0';
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s] %s%s", log_severity_colors[level], module, message, g_ColorReset);
        // set color depending on level
        // write module
    }
    else
    {
        count = sprintf(log, "%s %s%s", log_severity_colors[level], message, g_ColorReset);
        // set color depending on level
    }
    log[count] = '\0';

    spinlock_acquire(&debug_logs);
    debug_write_line(log, count);
    spinlock_release(&debug_logs);
}

