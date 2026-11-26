/*
 * File: debug.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

// #include "task/threading/spinlock/spinlock.h"
// #include "time/timer.h"
#include "trace.h"
#include "kerrno.h"
#include "panic.h"
#include "sync/spinlock.h"
#include "console/console.h"
#include <types.h>

// #if DEBUG
#define MIN_LOG_LEVEL LVL_DEBUG
// #else
// #define MIN_LOG_LEVEL LVL_INFO
// #endif

typedef enum
{
    DEBUG_LVL_DEBUG = 0,
    DEBUG_LVL_INFO = 1,
    DEBUG_LVL_WARN = 2,
    DEBUG_LVL_ERROR = 3,
    DEBUG_LVL_CRITICAL = 4
} debug_level_t;

#define NO_MODULE "\0"

extern spinlock_t debug_logs;

void logfl(const char *module, debug_level_t level, const char *fmt, ...);
void logfl_args(const char *module, debug_level_t level, const char *fmt, va_list args);
void logf(const char *module, debug_level_t level, const char *fmt, ...);
void logf_args(const char *module, debug_level_t level, const char *fmt, va_list args);

void __hexdump(void *ptr, size_t len, size_t align);
#define hexdump(ptr, len, align)      __hexdump(ptr, len, align);

// from VFS/vfs.h/c
// extern bool vfs_init_is_done;

#define log_debug(module, fmt, ...)   logfl(module, DEBUG_LVL_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(module, fmt, ...)    logfl(module, DEBUG_LVL_INFO, fmt, ##__VA_ARGS__)
#define log_warn(module, fmt, ...)    logfl(module, DEBUG_LVL_WARNING, fmt, ##__VA_ARGS__)
#define log_err(module, fmt, ...)     logfl(module, DEBUG_LVL_ERROR, fmt, ##__VA_ARGS__)
#define log_crit(module, fmt, ...)    logfl(module, DEBUG_LVL_CRITICAL, fmt, ##__VA_ARGS__)

#define trace_debug(module, fmt, ...) console_write_full(CON_WRITE_LEVEL_DEBUG, LVL_DEBUG, __FILE__, __FUNCTION__, __LINE__, module ": " fmt, ##__VA_ARGS__)
#define trace_info(module, fmt, ...)  console_write_full(CON_WRITE_LEVEL_NORMAL, LVL_INFO, __FILE__, __FUNCTION__, __LINE__, module ": " fmt, ##__VA_ARGS__)
#define trace_warn(module, fmt, ...)  console_write_full(CON_WRITE_LEVEL_NORMAL, LVL_WARNING, __FILE__, __FUNCTION__, __LINE__, module ": " fmt, ##__VA_ARGS__)
#define trace_err(module, fmt, ...)   console_write_full(CON_WRITE_LEVEL_NORMAL, LVL_ERROR, __FILE__, __FUNCTION__, __LINE__, module ": " fmt, ##__VA_ARGS__)
#define trace_crit(module, fmt, ...)  console_write_full(CON_WRITE_LEVEL_NORMAL, LVL_CRITICAL, __FILE__, __FUNCTION__, __LINE__, module ": " fmt, ##__VA_ARGS__)
#define ENTER_FUNC(args, ...)         console_write_full(CON_WRITE_LEVEL_ENTER_FUNC, LVL_FUNC_ENTER, __FILE__, __FUNCTION__, __LINE__, MODULE ": %s(" args ")", __FUNCTION__, ##__VA_ARGS__);

#define FUNC_NOT_IMPLEMENTED()                                       \
    KERRNO_NO_RETURN(ENOSYS, "%s is not implemented", __FUNCTION__); \
    KERNEL_PANIC(MODULE, "%s is not implemented", __FUNCTION__);

/// @brief this is called if the fix is quick and dirty or if a fix isn't there yet.
#define IMPLEMENTED_QUICK_AND_DIRTY()                                                           \
    KERRNO_NO_RETURN(KERRNO_DONT_DO_THAT, "this is a quick and dirty fix in %s", __FUNCTION__); \
    KERNEL_PANIC(MODULE, "this is a quick and dirty fix in %s", __FUNCTION__);

// #define KERNEL_PANIC(module, ...) panic(module, __FILE__, __LINE__, __VA_ARGS__)

