/*
 * File: assert.h
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "libs/stdio.h"
#include "task/signal.h"

#define ASSERT(expr, msg, ...)                                                                                      \
    if ((expr) == 0)                                                                                                  \
    {                                                                                                               \
        fprintf(VFS_FD_DEBUG, "proc: %s:%u: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __FUNCTION__, #expr); \
        fprintf(VFS_FD_DEBUG, msg, __VA_ARGS__);                                                                    \
        KernelPanic("ASSERT", "assert failed");                                                                     \
    }
