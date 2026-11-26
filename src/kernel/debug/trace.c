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
#include "console/console.h"

#include "memory.h"

#define NEW_BUFFER(name, size) \
    char name[(size)];         \
    memset(name, 0, (size));

static const char *const LogLevelSymbol[] =
    {
        [LVL_FUNC_ENTER] = "ENTER",
        [LVL_DEBUG] = "DEBUG",
        [LVL_INFO] = "INFO",
        [LVL_WARNING] = "WARNING",
        [LVL_FIX] = "FIX_FOUND",
        [LVL_ERROR] = "ERR",
        [LVL_CRITICAL] = "CRIT",
        [LVL_BUG] = "BUG",
};
// static debug_level_t LogLevelDebugLevel[] =
//     {
//         [LVL_FUNC_ENTER] = LVL_INFO,
//         [LVL_DEBUG] = 0,
//         [LVL_INFO] = LVL_INFO,
//         [LVL_WARNING] = LVL_WARNING,
//         [LVL_FIX] = LVL_INFO,
//         [LVL_ERROR] = LVL_ERROR,
//         [LVL_CRITICAL] = LVL_CRITICAL,
//         [LVL_BUG] = LVL_CRITICAL,
// };

spinlock_t trace_lock = {0};

// INTERNAL_INLINE void get_time(uint64_t *out_ns, uint64_t *out_ms, uint32_t *out_sec, uint32_t *out_min, uint32_t *out_hour)
// {
//     uint64_t ns = 0 /* timer_now_ns() */;
//     if (ns == 0)
//     {
//         *out_ns = 0;
//         *out_ms = 0;
//         *out_sec = 0;
//         *out_min = 0;
//         *out_hour = 0;
//         return;
//     }
//     uint64_t ms = NANOSEC_TO_MILLISEC(ns) % 1000;
//     uint32_t sec = NANOSEC_TO_SEC(ns);
//     uint32_t min = sec / 60;
//     uint32_t hour = min / 60;

//     *out_ns = ns;
//     *out_ms = ms % 1000;
//     *out_sec = sec % 60;
//     *out_min = min % 60;
//     *out_hour = hour % 24;
// }

// INTERNAL_INLINE void get_time_string(char *line)
// {
//     uint64_t ns = timer_now_ns();
//     if (ns == 0)
//     {
//         int count = sprintf(line, "[0:0:0.0]");
//         line[count] = '\0';
//         return;
//     }
//     uint32_t ms = NANOSEC_TO_MILLISEC(ns);
//     uint64_t sec = timer_now_sec();
//     uint32_t min = sec / 60;
//     uint32_t hour = min / 60;

//     int count = sprintf(line, "[%u:%u:%u.%u]", hour % 24, min % 60, sec % 60, ms % 1000);
//     line[count] = '\0';
// }

// INTERNAL_INLINE void get_ids(char *id_log)
// {
//     int count = 0;
//     cpu_t *cpu = cpu_arch_get_current();
//     if (cpu != NULL && cpu->current != NULL && (cpu->current->proc != NULL || cpu->current_proc != NULL))
//     {
//         if (cpu->current->proc)
//         {
//             count = sprintf(id_log, "%u.%u.%u", cpu->arch_id, cpu->current->tid, cpu->current->proc->pid);
//         }
//         else
//         {
//             count = sprintf(id_log, "%u.%u.%u", cpu->arch_id, cpu->current->tid, cpu->current_proc->pid);
//         }
//     }
//     else if (cpu != NULL && cpu->current != NULL)
//     {
//         count = sprintf(id_log, "%u.%u.N", cpu->arch_id, cpu->current->tid);
//     }
//     else if (cpu != NULL)
//     {
//         count = sprintf(id_log, "%u.N.N", cpu->arch_id);
//     }
//     else
//     {
//         count = sprintf(id_log, "P.SMP");
//     }
//     id_log[count] = '\0';
// }

// void trace(fd_t file, trace_level_t level, char *fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);

//     int count;
//     NEW_BUFFER(fmt_log, 255);
//     count = vsprintf(fmt_log, fmt, args);
//     fmt_log[count] = '\0';

//     const char *symbol = LogLevelSymbol[level];

//     NEW_BUFFER(time, 50);
//     get_time_string(time);

//     NEW_BUFFER(log, 300);
//     count = sprintf(log, "%s %s: %s", time, symbol, fmt_log);
//     log[count] = '\0';

//     spinlock_acquire(&trace_lock);
//     logfl(NO_MODULE, LogLevelDebugLevel[level], log);
//     // vfs_write(file, log, count);
//     va_end(args);
//     spinlock_release(&trace_lock);
// }

// void trace_with_id(fd_t file, trace_level_t level, const char *module, char *fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);

//     int count;
//     NEW_BUFFER(fmt_log, 255);
//     count = vsprintf(fmt_log, fmt, args);
//     fmt_log[count] = '\0';

//     const char *symbol = LogLevelSymbol[level];

//     char id_log[20];
//     get_ids(id_log);

//     NEW_BUFFER(time, 50);
//     get_time_string(time);

//     NEW_BUFFER(log, 300);
//     if (*module != '\0')
//     {
//         count = sprintf(log, "%s [%s] %s, %s: %s", time, id_log, symbol, module, fmt_log);
//     }
//     else
//     {
//         count = sprintf(log, "%s [%s] %s, : %s", time, id_log, symbol, fmt_log);
//     }
//     log[count] = '\0';

//     spinlock_acquire(&trace_lock);
//     logfl(NO_MODULE, LogLevelDebugLevel[level], log);
//     // vfs_write(file, log, count);
//     va_end(args);
//     spinlock_release(&trace_lock);
// }

void __hexdump(void *ptr, size_t len, size_t size)
{
    spinlock_acquire(&trace_lock);
    pr_debug("========= HEXDUMP =========");
    pr_debug("hexdump at %p length %u", ptr, len);

    int count;
    NEW_BUFFER(fmt_log, 255);

    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i % size) == 0)
        {
            if (i != 0)
            {
                fmt_log[count] = '\0';
                pr_debug("%s", fmt_log);
                memset(fmt_log, 0, sizeof(fmt_log));
            }
            count = sprintf(fmt_log, "%04x: %02x", i, p[i]);
        }
        else
        {
            count = sprintf(fmt_log, "%s %02x", fmt_log, p[i]);
        }
    }
    fmt_log[count] = '\0';
    pr_debug("%s", fmt_log);
    spinlock_release(&trace_lock);
}
