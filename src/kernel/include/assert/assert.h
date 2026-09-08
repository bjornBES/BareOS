/*
 * File: assert.h
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "stdio.h"
#include "panic.h"

#define ASSERT_RETURN(expr, return_value, ...)                                            \
    if ((expr) == 0)                                                                      \
    {                                                                                     \
        fprintf(DEBUG_FD, "proc: %s:%u: %s: Assertion '%s' failed\n", __FILE__, __LINE__, \
                __FUNCTION__, #expr);                                                     \
        fprintf(DEBUG_FD, __VA_ARGS__);                                                   \
        return return_value;                                                              \
        KERNEL_PANIC("ASSERT", "assert failed");                                          \
    }

#define ASSERT(expr, ...)                                                                 \
    if ((expr) == 0)                                                                      \
    {                                                                                     \
        fprintf(DEBUG_FD, "proc: %s:%u: %s: Assertion '%s' failed\n", __FILE__, __LINE__, \
                __FUNCTION__, #expr);                                                     \
        fprintf(DEBUG_FD, __VA_ARGS__);                                                   \
        KERNEL_PANIC("ASSERT", "assert failed");                                          \
    }
