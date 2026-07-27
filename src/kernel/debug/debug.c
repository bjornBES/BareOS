/*
 * File: debug.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "debug.h"
#include "stdio.h"
#include "task/threading/spinlock/spinlock.h"
#include <printf_driver/printf.h>
#include "kernel/debug.h"
#include "kernel/irq.h"

static const char *const g_LogSeverityColors[] =
    {
        [LVL_DEBUG] = "\033[2;37m",
        [LVL_INFO] = "\033[37m",
        [LVL_WARN] = "\033[1;33m",
        [LVL_ERROR] = "\033[1;31m",
        [LVL_CRITICAL] = "\033[1;37;41m",
};

static const char *const g_ColorReset = "\033[0m";

spinlock_t debug_logs = {0};

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

    irq_arch_disable();
    spinlock_acquire(&debug_logs);
    char log[255];
    int count;
    memset(log, 0, 255);
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s] ", g_LogSeverityColors[level], module);
        // set color depending on level
        // write module
        debug_write_line(log, count);
    }
    else
    {
        count = sprintf(log, "%s", g_LogSeverityColors[level]);
        // set color depending on level
        debug_write_line(log, count);
    }
    memset(log, 0, 255);
    count = vsprintf(log, fmt, args); // write text
    debug_write_line(log, count);

    count = sprintf(log, "%s\n", g_ColorReset); // write text
    debug_write_line(log, count);
    spinlock_release(&debug_logs);
    irq_arch_enable();
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

    irq_arch_disable();
    spinlock_acquire(&debug_logs);

    char log[255];
    int count;
    memset(log, 0, 255);
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s]", g_LogSeverityColors[level], module);
        // set color depending on level
        // write module
        debug_write_line(log, count);
    }
    else
    {
        count = sprintf(log, "%s", g_LogSeverityColors[level]);
        // set color depending on level
        debug_write_line(log, count);
    }
    memset(log, 0, 255);
    count = vsprintf(log, fmt, args); // write text
    debug_write_line(log, count);

    count = sprintf(log, "%s", g_ColorReset); // write text
    debug_write_line(log, count);
    spinlock_release(&debug_logs);
    irq_arch_enable();
}

void debug_enter_func(const char *module, const char *function, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    irq_arch_disable();
    spinlock_acquire(&debug_logs);

    char log[255];
    int count;
    memset(log, 0, 255);
    if (*module != '\0')
    {
        count = sprintf(log, "%s[%s] Enter %s(", g_LogSeverityColors[LVL_DEBUG], module, function);
        debug_write_line(log, count);
    }
    else
    {
        count = sprintf(log, "%sEnter %s(", g_LogSeverityColors[LVL_DEBUG], function);
        debug_write_line(log, count);
    }
    memset(log, 0, 255);
    count = vsprintf(log, fmt, args); // write text
    debug_write_line(log, count);

    count = sprintf(log, ")%s\n", g_ColorReset); // write text
    debug_write_line(log, count);
    spinlock_release(&debug_logs);
    irq_arch_enable();
    va_end(args);
}

void strlogf(DebugLevel level, const char *str)
{
    if (level < MIN_LOG_LEVEL)
    {
        return;
    }

    fputs(g_LogSeverityColors[level], VFS_FD_DEBUG); // set color depending on level
    fprintf(VFS_FD_DEBUG, str);                      // write text
    fputs(g_ColorReset, VFS_FD_DEBUG);               // reset format
    fputs("\n", VFS_FD_DEBUG);                       // newline
}

void write_error(DebugLevel level, const char *module, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vprintf(VFS_FD_STDERR, fmt, args);

    fputs(g_LogSeverityColors[level], VFS_FD_DEBUG); // set color depending on level
    fprintf(VFS_FD_DEBUG, "[%s] ", module);          // write module
    vprintf(VFS_FD_DEBUG, fmt, args);                // write text
    fputs(g_ColorReset, VFS_FD_DEBUG);               // reset format
    fputs("\n", VFS_FD_DEBUG);                       // newline

    va_end(args);
}
