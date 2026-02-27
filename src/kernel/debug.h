/*
 * File: debug.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <debug.h>
#include <stdio.h>
#define MIN_LOG_LEVEL LVL_DEBUG

#ifndef DEBUGLEVELDEF
#define DEBUGLEVELDEF 1
typedef enum {
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 2,
    LVL_ERROR = 3,
    LVL_CRITICAL = 4
} DebugLevel;
#endif

void logf(const char* module, DebugLevel level, const char* fmt, ...);
void strlogf(DebugLevel level, const char* str);

#define log_debug(module, ...)          logf(module, LVL_DEBUG, __VA_ARGS__)
#define log_info(module, ...)           logf(module, LVL_INFO, __VA_ARGS__)
#define log_warn(module, ...)           logf(module, LVL_WARN, __VA_ARGS__)
#define log_err(module, ...)            logf(module, LVL_ERROR, __VA_ARGS__)
#define log_crit(module, ...)           logf(module, LVL_CRITICAL, __VA_ARGS__)

#define _log_debug(str)          strlogf(LVL_DEBUG, str)
#define _log_info(str)           strlogf(LVL_INFO, str)
#define _log_warn(str)           strlogf(LVL_WARN, str)
#define _log_err(str)            strlogf(LVL_ERROR, str)
#define _log_crit(str)           strlogf(LVL_CRITICAL, str)

#define FUNC_NOT_IMPLEMENTED(module, func) log_err(module, "%s: Not implemented", func)

void write_error(DebugLevel level, const char* module, const char* fmt, ...);
