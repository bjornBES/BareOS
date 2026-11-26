/*
 * File: trace.h
 * File Created: 30 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef enum trace_level
{
    LVL_FUNC_ENTER = 0,
    LVL_DEBUG,
    LVL_INFO,
    LVL_SCHED,
    LVL_NOTICE,
    LVL_WARNING,
    LVL_FIX,  // fix
    LVL_ERROR,  // error
    LVL_CRITICAL, // crit
    LVL_ALERT,
    LVL_EMERG,
    LVL_BUG,
} trace_level_t;

// void trace(fd_t file, trace_level_t level, char *fmt, ...);
// void trace_with_id(fd_t file, trace_level_t level, const char *module, char *fmt, ...);
// void trace_enter_func(fd_t file, const char *module, trace_level_t level, const char *function, char *fmt, ...);
