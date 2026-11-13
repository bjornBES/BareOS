/*
 * File: syscall.h
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "syscallt.h"

status_t syscall_get_handler(uint32_t sys_number, syscall_function_info_t **out_info);
status_t syscall_handler_func(syscall_info_t *info, syscall_function_info_t *func_info, intptr_t *out_ret);

#define DEFINE_ARG(type, name, index)                \
    type name = (type)(intptr_t)(info->args[index - 1]);

#define SYSCALL_DEFINEN(func)                                    \
    status_t sys_##func(syscall_info_t *info, intptr_t *out_ret)

#define SYSCALL_DEFINE0(func)      \
    SYSCALL_DEFINEN(func)          \
    {                              \
        ENTER_FUNC("%p", out_ret); \
        return func(out_ret);      \
    }

#define SYSCALL_DEFINE1(func, t1)            \
    SYSCALL_DEFINEN(func)                    \
    {                                        \
        DEFINE_ARG(t1, arg1, 1);             \
        ENTER_FUNC("%p, %p", out_ret, arg1); \
        return func(out_ret, arg1);          \
    }

#define SYSCALL_DEFINE2(func, t1, t2)                  \
    SYSCALL_DEFINEN(func)                              \
    {                                                  \
        DEFINE_ARG(t1, arg1, 1);                       \
        DEFINE_ARG(t2, arg2, 2);                       \
        ENTER_FUNC("%p, %p, %p", out_ret, arg1, arg2); \
        return func(out_ret, arg1, arg2);              \
    }

#define SYSCALL_DEFINE3(func, t1, t2, t3)                        \
    SYSCALL_DEFINEN(func)                                        \
    {                                                            \
        DEFINE_ARG(t1, arg1, 1);                                 \
        DEFINE_ARG(t2, arg2, 2);                                 \
        DEFINE_ARG(t3, arg3, 3);                                 \
        return func(out_ret, arg1, arg2, arg3);                  \
    }

#define SYSCALL_DEFINE4(func, t1, t2, t3, t4)                              \
    SYSCALL_DEFINEN(func)                                                  \
    {                                                                      \
        DEFINE_ARG(t1, arg1, 1);                                           \
        DEFINE_ARG(t2, arg2, 2);                                           \
        DEFINE_ARG(t3, arg3, 3);                                           \
        DEFINE_ARG(t4, arg4, 4);                                           \
        ENTER_FUNC("%p, %p, %p, %p, %p", out_ret, arg1, arg2, arg3, arg4); \
        return func(out_ret, arg1, arg2, arg3, arg4);                      \
    }

#define SYSCALL_DEFINE6(func, t1, t2, t3, t4, t5, t6)                                          \
    SYSCALL_DEFINEN(func)                                                                      \
    {                                                                                          \
        DEFINE_ARG(t1, arg1, 1);                                                               \
        DEFINE_ARG(t2, arg2, 2);                                                               \
        DEFINE_ARG(t3, arg3, 3);                                                               \
        DEFINE_ARG(t4, arg4, 4);                                                               \
        DEFINE_ARG(t5, arg5, 5);                                                               \
        DEFINE_ARG(t6, arg6, 6);                                                               \
        ENTER_FUNC("%p, %p, %p, %p, %p, %p, %p", out_ret, arg1, arg2, arg3, arg4, arg5, arg6); \
        return func(out_ret, arg1, arg2, arg3, arg4, arg5, arg6);                              \
    }
