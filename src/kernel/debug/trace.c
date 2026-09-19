/*
 * File: trace.c
 * File Created: 30 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "debug/trace.h"
#include "stdio.h"
#include "sync/spinlock.h"
#include <printf_driver/printf.h>
#include "debug/debug.h"
#include "asm/irq_arch.h"
#include "asm/cpu_arch.h"
// #include "drivers/CMOS/CMOS.h"
#include "timer/timer.h"

#include "memory.h"

#define NEW_BUFFER(name, size) \
    char name[(size)];         \
    memset(name, 0, (size));

static const char *const LogLevelSymbol[] =
    {
        [FUNC_ENTER] = "ENTER",
        [LVL_WARNING] = "WARNING",
        [LVL_FIX] = "FIX_FOUND",
        [LVL1] = "DEBUG",
        [LVL2] = "INFO",
        [LVL3] = "",
        [LVL4] = "",
        [LVL5] = "ERR",
};

spinlock_t trace_lock = {0};

static inline void get_time(uint64_t *out_ns, uint64_t *out_ms, uint32_t *out_sec, uint32_t *out_min, uint32_t *out_hour)
{
    uint64_t ns = 0/* timer_now_ns() */;
    if (ns == 0)
    {
        *out_ns = 0;
        *out_ms = 0;
        *out_sec = 0;
        *out_min = 0;
        *out_hour = 0;
        return;
    }
    uint64_t ms = NANOSEC_TO_MILLISEC(ns) % 1000;
    uint32_t sec = NANOSEC_TO_SEC(ns);
    uint32_t min = sec / 60;
    uint32_t hour = min / 60;

    *out_ns = ns;
    *out_ms = ms % 1000;
    *out_sec = sec % 60;
    *out_min = min % 60;
    *out_hour = hour % 24;
}

static inline void get_time_string(char *line)
{
    uint64_t ns = 0/* timer_now_ns() */;
    if (ns == 0)
    {
        line[0] = '\0';
        return;
    }
    uint32_t ms = NANOSEC_TO_MILLISEC(ns) % 1000;
    uint64_t sec = 0/* timer_now_sec() */;
    uint32_t min = sec / 60;
    uint32_t hour = min / 60;

    int count = sprintf(line, "[%u:%u:%u.%u]", hour % 24, min % 60, sec % 60, ms);
    line[count] = '\0';
}

static inline void get_ids(char *id_log)
{
    int count = 0;
    cpu_t *cpu = cpu_arch_get_current();
    /* if (cpu != NULL && cpu->current != NULL && cpu->current->proc != NULL)
    {
        count = sprintf(id_log, "%u.%u.%u", cpu->apic_id, cpu->current->tid, cpu->current->proc->pid);
    }
    else if (cpu != NULL && cpu->current != NULL)
    {
        count = sprintf(id_log, "%u.%u", cpu->apic_id, cpu->current->tid);
    }
    else  */if (cpu != NULL)
    {
        count = sprintf(id_log, "%u", cpu->apic_id);
    }
    else
    {
        count = 0;
    }
    id_log[count] = '\0';
}

void trace_enter_func(fd_t file, const char *module, trace_level_t level, const char *function, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int count;
    NEW_BUFFER(fmt_log, 255);
    count = vsprintf(fmt_log, fmt, args);
    fmt_log[count] = '\0';

    const char *symbol = LogLevelSymbol[level];

    NEW_BUFFER(time, 50);
    get_time_string(time);

    NEW_BUFFER(id_log, 20);
    get_ids(id_log);

    NEW_BUFFER(log, 300);
    count = sprintf(log, "%s [%s] %s: %s(%s)\n", time, id_log, symbol, function, fmt_log);
    log[count] = '\0';

    spinlock_acquire(&debug_logs);
    spinlock_acquire(&trace_lock);
    log_info(NO_MODULE, log);
    // vfs_write(file, log, count);
    va_end(args);
    spinlock_release(&trace_lock);
    spinlock_release(&debug_logs);
}

void trace(fd_t file, trace_level_t level, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int count;
    NEW_BUFFER(fmt_log, 255);
    count = vsprintf(fmt_log, fmt, args);
    fmt_log[count] = '\0';

    const char *symbol = LogLevelSymbol[level];

    NEW_BUFFER(time, 50);
    get_time_string(time);

    NEW_BUFFER(log, 300);
    count = sprintf(log, "%s %s: %s", time, symbol, fmt_log);
    log[count] = '\0';

    spinlock_acquire(&debug_logs);
    spinlock_acquire(&trace_lock);
    log_info(NO_MODULE, log);
    // vfs_write(file, log, count);
    va_end(args);
    spinlock_release(&trace_lock);
    spinlock_release(&debug_logs);
}

void trace_with_id(fd_t file, trace_level_t level, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int count;
    NEW_BUFFER(fmt_log, 255);
    count = vsprintf(fmt_log, fmt, args);
    fmt_log[count] = '\0';

    const char *symbol = LogLevelSymbol[level];

    char id_log[20];
    get_ids(id_log);

    NEW_BUFFER(time, 50);
    get_time_string(time);

    NEW_BUFFER(log, 300);
    count = sprintf(log, "%s [%s] %s: %s", time, id_log, symbol, fmt_log);
    log[count] = '\0';

    spinlock_acquire(&debug_logs);
    spinlock_acquire(&trace_lock);
    log_info(NO_MODULE, log);
    // vfs_write(file, log, count);
    va_end(args);
    spinlock_release(&trace_lock);
    spinlock_release(&debug_logs);
}
