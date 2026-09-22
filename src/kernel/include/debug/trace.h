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

typedef enum
{
    FUNC_ENTER = 0,
    LVL_WARNING = 0xEE,
    LVL_FIX = 0xEF, // fix
    LVL1 = 0xF0, // debug
    LVL2 = 0xF1, // info
    LVL3 = 0xF2, //
    LVL4 = 0xF3,
    LVL5 = 0xF4
} trace_level_t;

void trace(fd_t file, trace_level_t level, char *fmt, ...);
void trace_with_id(fd_t file, trace_level_t level, const char *module, char *fmt, ...);
void trace_enter_func(fd_t file, const char *module, trace_level_t level, const char *function, char *fmt, ...);