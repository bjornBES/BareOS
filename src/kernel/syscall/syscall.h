/*
 * File: syscall.h
 * File Created: 22 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

typedef struct
{
#ifndef __i686__
    uint64_t pc;
    uint64_t bp;
    uint64_t sp;
    uint64_t di;
    uint64_t si;
    uint64_t bx;
    uint64_t dx;
    uint64_t cx;
    uint64_t ax;
#else
    uint32_t pc;
    uint32_t bp;
    uint32_t sp;
    uint32_t di;
    uint32_t si;
    uint32_t bx;
    uint32_t dx;
    uint32_t cx;
    uint32_t ax;
#endif
    uint16_t ds, cs;
    uint16_t ss;
#ifndef __i686__
    uint64_t flags;
#else
    uint32_t flags;
#endif
} syscall_registers;
typedef int (*syscall_handler)(syscall_registers *regs);

#define SYSCALL_DEFINE0(func, ret)             \
    ret sys_##func##(syscall_registers * regs) \
    {                                          \
        return func();                         \
    }

#define SYSCALL_DEFINE1(func, ret, t1)       \
    ret sys_##func(syscall_registers *regs)  \
    {                                        \
        t1 arg1 = (t1)(uintptr_t)(regs->di); \
        return func(arg1);                   \
    }

#define SYSCALL_DEFINE2(func, ret, t1, t2)   \
    ret sys_##func(syscall_registers *regs)  \
    {                                        \
        t1 arg1 = (t1)(uintptr_t)(regs->di); \
        t2 arg2 = (t2)(uintptr_t)(regs->si); \
        return func(arg1, arg2);             \
    }

#define SYSCALL_DEFINE3(func, ret, t1, t2, t3) \
    ret sys_##func(syscall_registers *regs)    \
    {                                          \
        t1 arg1 = (t1)(uintptr_t)(regs->di);   \
        t2 arg2 = (t2)(uintptr_t)(regs->si);   \
        t3 arg3 = (t3)(uintptr_t)(regs->dx);   \
        return func(arg1, arg2, arg3);         \
    }

#define SYSCALL_ENTRY(number, name) [number] = sys_##name

void syscall_init();
