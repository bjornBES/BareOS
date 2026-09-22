/*
 * File: thread.h
 * File Created: 19 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "threadt.h"

#include <types.h>

#define THREAD_CALL_FUNC(func) t_##func

#define THREAD_WARPER(func, arg_type)               \
    uintptr_t THREAD_CALL_FUNC(func)(uintptr_t arg) \
    {                                               \
        func((arg_type)arg);                        \
    }
#define THREAD_WARPER_NO_RETURN(func, arg_type)     \
    uintptr_t THREAD_CALL_FUNC(func)(uintptr_t arg) \
    {                                               \
        func((arg_type)arg);                        \
        return KERRNO_SUCCESSES;                    \
    }

#define THREAD_WARPER_NO_RETURN_ARG(func)         \
    uintptr_t THREAD_CALL_FUNC(func)(uintptr_t _) \
    {                                             \
        func();                                   \
        return KERRNO_SUCCESSES;                  \
    }

void thread_set_priority(thread_t *t, int priority);
thread_t *thread_create_kernel(thread_func_t entry, uintptr_t arg, uint32_t flags);
