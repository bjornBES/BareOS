/*
 * File: console.h
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "consolet.h"
#include "kernel_levels.h"

#include <defs.h>

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define pr_err(fmt, ...)                                                                                                \
    console_write_full(LVL_ERROR, CON_WRITE_LEVEL_NORMAL, __FILE__, __FUNCTION__, __LINE__, pr_fmt(fmt), ##__VA_ARGS__)

#define pr_warn(fmt, ...)                                                                                                 \
    console_write_full(LVL_WARNING, CON_WRITE_LEVEL_NORMAL, __FILE__, __FUNCTION__, __LINE__, pr_fmt(fmt), ##__VA_ARGS__)

#define pr_info(fmt, ...)                                                                                              \
    console_write_full(LVL_INFO, CON_WRITE_LEVEL_NORMAL, __FILE__, __FUNCTION__, __LINE__, pr_fmt(fmt), ##__VA_ARGS__)

#define pr_debug(fmt, ...)                                                                                             \
    console_write_full(LVL_DEBUG, CON_WRITE_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, pr_fmt(fmt), ##__VA_ARGS__)

static inline bool console_is_usable(console_t *con, short flags, bool use_atomic)
{
    if (!(flags & CON_ENABLED))
    {
        return false;
    }

    if (flags & CON_SUSPENDED)
    {
        return false;
    }

    if (flags & CON_NBCON)
    {
        if (use_atomic)
        {
        }
    }
    else
    {
        if (!con->write)
        {
            return false;
        }
    }

    // if (!cpu_online(raw_smp_processor_id()) && !(flags & CON_ANYTIME))
    // {
    //     return false;
    // }

    return true;
}

status_t console_register(console_t *cons);
status_t console_write_full(con_write_level_t write_level, trace_level_t log_level, const char *file, const char *func, uint32_t line, const char *message, ...);
status_t console_write(con_write_level_t write_level, trace_level_t log_level, const char *message, ...);
status_t console_write_arg(con_write_level_t write_level, trace_level_t log_level, const char *message, va_list args);
bool console_is_registered(const console_t *con);
