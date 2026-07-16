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
#include "stdio.h"
#include "task/threading/spinlock/spinlock.h"
#include <stdbool.h>
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

void logfl(const char *module, DebugLevel level, const char *fmt, ...);
void logf(const char *module, DebugLevel level, const char *fmt, ...);
void strlogf(DebugLevel level, const char *str);

#if DEBUG
#define log_debug(module, ...) logfl(module, LVL_DEBUG, __VA_ARGS__)
#else
#define log_debug(module, ...) __asm__("nop")
#endif
#define log_info(module, ...) logfl(module, LVL_INFO, __VA_ARGS__)
#define log_warn(module, ...) logfl(module, LVL_WARN, __VA_ARGS__)
#define log_err(module, ...) logfl(module, LVL_ERROR, __VA_ARGS__)
#define log_crit(module, ...) logfl(module, LVL_CRITICAL, __VA_ARGS__)

#define _log_debug(str) strlogf(LVL_DEBUG, str)
#define _log_info(str) strlogf(LVL_INFO, str)
#define _log_warn(str) strlogf(LVL_WARN, str)
#define _log_err(str) strlogf(LVL_ERROR, str)
#define _log_crit(str) strlogf(LVL_CRITICAL, str)

#define ENTER_FUNC(module, args, ...)                       \
    {                                                       \
        logf(module, LVL_DEBUG, "Enter %s(", __FUNCTION__); \
        logf("\0", LVL_DEBUG, args, __VA_ARGS__);           \
        logfl("\0", LVL_DEBUG, ")");                        \
    }
#define FUNC_NOT_IMPLEMENTED(module) log_err(module, "%s: Not implemented", __FUNCTION__)

extern void arch_breakpoint();

extern bool __unreachable();

void write_error(DebugLevel level, const char *module, const char *fmt, ...);
