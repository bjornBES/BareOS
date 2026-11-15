/*
 * File: cmdline_init.h
 * File Created: 15 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>
#include <types.h>

typedef struct
{
    const char *key;
    status_t (*handler)(const char *value); // value NULL for bare flags
    bool early;
} cmdline_param_t;

#define CMDLINE_PARAM(k, fn, e)                                                              \
    static USED SECTION(".cmdline_params") const cmdline_param_t __cmdline_##fn = {k, fn, e}

