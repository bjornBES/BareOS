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
#include "signal/signal.h"

#define ASSERT(expr, ...)                                                                                           \
    if ((expr) == 0)                                                                                                \
    {                                                                                                               \
        fprintf(VFS_FD_DEBUG, "proc: %s:%u: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __FUNCTION__, #expr); \
        fprintf(VFS_FD_DEBUG, __VA_ARGS__);                                                                         \
        KernelPanic("ASSERT", "assert failed");                                                                     \
    }
