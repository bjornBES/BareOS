/*
 * File: syscall.h
 * File Created: 22 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 23:28:09
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include "pledge.h"
#include "kernel/registers.h"
#include "kernel/syscall.h"

struct __syscall_function_info_t;

typedef uintptr_t (*syscall_handler)(syscall_info *info, struct __syscall_function_info_t *sys_info);

typedef struct __syscall_function_info_t
{
    int number;
    syscall_handler handler;
    bool call_dose_return;
    pledge_flags_t pledge;
} syscall_function_info;

#define SYSCALL_DEFINE0(func)                                                 \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        return (uintptr_t)func();                                             \
    }
#define SYSCALL_DEFINE0_REG(func)                                             \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        return (uintptr_t)func(info);                                         \
    }

#define SYSCALL_DEFINE1(func, t1)                                             \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        return (uintptr_t)func(arg1);                                         \
    }

#define SYSCALL_DEFINE2(func, t1, t2)                                         \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        t2 arg2 = (t2)(uintptr_t)(info->arg2);                                \
        return (uintptr_t)func(arg1, arg2);                                   \
    }

#define SYSCALL_DEFINE3(func, t1, t2, t3)                                     \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        t2 arg2 = (t2)(uintptr_t)(info->arg2);                                \
        t3 arg3 = (t3)(uintptr_t)(info->arg3);                                \
        return (uintptr_t)func(arg1, arg2, arg3);                             \
    }
#define SYSCALL_DEFINE3_REG(func, t1, t2, t3)                                 \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        t2 arg2 = (t2)(uintptr_t)(info->arg2);                                \
        t3 arg3 = (t3)(uintptr_t)(info->arg3);                                \
        return (uintptr_t)func(arg1, arg2, arg3, info);                       \
    }

#define SYSCALL_DEFINE6(func, t1, t2, t3, t4, t5, t6)                         \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        t2 arg2 = (t2)(uintptr_t)(info->arg2);                                \
        t3 arg3 = (t3)(uintptr_t)(info->arg3);                                \
        t4 arg4 = (t4)(uintptr_t)(info->arg4);                                \
        t5 arg5 = (t5)(uintptr_t)(info->arg5);                                \
        t6 arg6 = (t6)(uintptr_t)(info->arg6);                                \
        return (uintptr_t)func(arg1, arg2, arg3, arg4, arg5, arg6);           \
    }

#define SYSCALL_DEFINE0_PLEDGE(func)                                              \
    uintptr_t sys_##func##(syscall_info * regs, syscall_function_info * sys_info) \
    {                                                                             \
        return (uintptr_t)func(sys_info->pledge);                                 \
    }

#define SYSCALL_DEFINE1_PLEDGE(func, t1)                                      \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        return (uintptr_t)func(arg1, sys_info->pledge);                       \
    }

#define SYSCALL_DEFINE2_PLEDGE(func, t1, t2)                                  \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        t2 arg2 = (t2)(uintptr_t)(info->arg2);                                \
        return (uintptr_t)func(arg1, arg2, sys_info->pledge);                 \
    }

#define SYSCALL_DEFINE3_PLEDGE(func, t1, t2, t3)                              \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info) \
    {                                                                         \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                \
        t2 arg2 = (t2)(uintptr_t)(info->arg2);                                \
        t3 arg3 = (t3)(uintptr_t)(info->arg3);                                \
        return (uintptr_t)func(arg1, arg2, arg3, sys_info->pledge);           \
    }

#define SYSCALL_DEFINE6_PLEDGE(func, t1, t2, t3, t4, t5, t6)                          \
    uintptr_t sys_##func(syscall_info *info, syscall_function_info *sys_info)         \
    {                                                                                 \
        t1 arg1 = (t1)(uintptr_t)(info->arg1);                                        \
        t2 arg2 = (t2)(uintptr_t)(info->arg2);                                        \
        t3 arg3 = (t3)(uintptr_t)(info->arg3);                                        \
        t4 arg4 = (t4)(uintptr_t)(info->arg4);                                        \
        t5 arg5 = (t5)(uintptr_t)(info->arg5);                                        \
        t6 arg6 = (t6)(uintptr_t)(info->arg6);                                        \
        return (uintptr_t)func(arg1, arg2, arg3, arg4, arg5, arg6, sys_info->pledge); \
    }

#define SYSCALL_ENTRY(number, name) [number] = sys_##name

void syscall_init();
