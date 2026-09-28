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
    LVL1 = 0x1, // debug
    LVL2 = 0x2, // info
    LVL3 = 0x3, //
    LVL4 = 0x4,
    LVL5 = 0x5,
    LVL_WARNING = 0xE0,
    LVL_FIX, // fix
    LVL_ERR, // error
    LVL_CRIT, // crit
    LVL_BUG = 0xFF,
} trace_level_t;

void trace(fd_t file, trace_level_t level, char *fmt, ...);
void trace_with_id(fd_t file, trace_level_t level, const char *module, char *fmt, ...);
void trace_enter_func(fd_t file, const char *module, trace_level_t level, const char *function, char *fmt, ...);