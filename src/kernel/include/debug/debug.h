/*
 * File: debug.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

// #include "task/threading/spinlock/spinlock.h"
// #include "trace.h"
// #include "time/timer.h"
#include "errno.h"
#include <types.h>

#if DEBUG
#define MIN_LOG_LEVEL LVL_DEBUG
#else
#define MIN_LOG_LEVEL LVL_INFO
#endif

#define NO_MODULE "\0"

#ifndef DEBUGLEVELDEF
#define DEBUGLEVELDEF 1

typedef enum
{
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 2,
    LVL_ERROR = 3,
    LVL_CRITICAL = 4
} DebugLevel;
#endif

// extern spinlock_t debug_logs;

void logfl(const char *module, DebugLevel level, const char *fmt, ...);
void logfl_args(const char *module, DebugLevel level, const char *fmt, va_list args);
void logf(const char *module, DebugLevel level, const char *fmt, ...);
void logf_args(const char *module, DebugLevel level, const char *fmt, va_list args);
void logfl_int(const char *module, DebugLevel level, const char *fmt, ...);
void logfl_int_args(const char *module, DebugLevel level, const char *fmt, va_list args);
void debug_enter_func(const char *module, const char *function, const char *fmt, ...);

// from VFS/vfs.h/c
// extern bool vfs_init_is_done;

#ifdef DEBUG
#define log_debug(module, ...)     logfl(module, LVL_DEBUG, __VA_ARGS__)
#define log_debug_int(module, ...) logfl_int(module, LVL_DEBUG, __VA_ARGS__)
#define log(module, ...)                         \
    {                                            \
        if (vfs_init_is_done == false)           \
        {                                        \
            log_debug(module, __VA_ARGS__);      \
        }                                        \
        else                                     \
        {                                        \
            trace_with_id(4, LVL1, __VA_ARGS__); \
        }                                        \
    }
#else
#define log_debug(module, ...) __asm__("nop")
#endif
#define log_info(module, ...)     logfl(module, LVL_INFO, __VA_ARGS__)
#define log_info_int(module, ...) logfl_int(module, LVL_INFO, __VA_ARGS__)
#define info(module, ...)                         \
    {                                            \
        if (vfs_init_is_done == false)           \
        {                                        \
            log_info(module, __VA_ARGS__);      \
        }                                        \
        else                                     \
        {                                        \
            trace_with_id(4, LVL2, __VA_ARGS__); \
        }                                        \
    }

#define log_warn(module, ...)     logfl(module, LVL_WARN, __VA_ARGS__)
#define log_warn_int(module, ...) logfl_int(module, LVL_WARN, __VA_ARGS__)
#define warn(module, ...)                         \
    {                                            \
        if (vfs_init_is_done == false)           \
        {                                        \
            log_warn(module, __VA_ARGS__);      \
        }                                        \
        else                                     \
        {                                        \
            trace_with_id(4, LVL_WARNING, __VA_ARGS__); \
        }                                        \
    }

#define log_err(module, ...)      logfl(module, LVL_ERROR, __VA_ARGS__)
#define log_err_int(module, ...)  logfl_int(module, LVL_ERROR, __VA_ARGS__)

#define log_crit(module, ...)     logfl(module, LVL_CRITICAL, __VA_ARGS__)
#define log_crit_int(module, ...) logfl_int(module, LVL_CRITICAL, __VA_ARGS__)

#define trace_1(module, ...)      logfl(module, LVL_DEBUG, __VA_ARGS__)


/* #define ENTER_FUNC(module, args, ...)                                                 \
    {                                                                                 \
        irq_arch_disable();                                                           \
        if (vfs_init_is_done == false)                                                \
        {                                                                             \
            debug_enter_func(module, __FUNCTION__, args, __VA_ARGS__);                \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            trace_enter_func(4, module, FUNC_ENTER, __FUNCTION__, args, __VA_ARGS__); \
        }                                                                             \
        irq_arch_enable();                                                            \
    } */
#define FUNC_NOT_IMPLEMENTED() \
    ERRNO_NO_RETURN(ENOSYS, "%s is not implemented", __FUNCTION__);

// #define KERNEL_PANIC(module, ...) panic(module, __FILE__, __LINE__, __VA_ARGS__)

