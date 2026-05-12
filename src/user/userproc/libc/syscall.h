/*
 * File: syscall.h
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <pledge_types.h>
#include <defs.h>
#include <syscall/syscall_number.h>
#include "types.h"

// raw
static inline uintptr_t syscall0(int number)
{
    uintptr_t result;
    inline_asm(
        "int 0x80" : "=a"(result) : "a"(number) : "memory");
    return result;
}

static inline uintptr_t syscall1(int number, uintptr_t arg1)
{
    uintptr_t result;
    inline_asm(
        "int 0x80" : "=a"(result) : "a"(number), "D"(arg1) : "memory");
    return result;
}
static inline uintptr_t syscall2(int number, uintptr_t arg1, uintptr_t arg2)
{
    uintptr_t result;
    inline_asm(
        "int 0x80" : "=a"(result) : "a"(number), "D"(arg1), "S"(arg2) : "memory");
    return result;
}

static inline uintptr_t syscall3(int number, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    uintptr_t result;
    inline_asm(
        "int 0x80" : "=a"(result) : "a"(number), "D"(arg1), "S"(arg2), "d"(arg3) : "memory", "rcx", "r11");
    return result;
}

static inline uintptr_t syscall6(int number, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6)
{
    uintptr_t result;
    inline_asm(
        "mov r9, %[arg6]\n\t"
        "mov r8, %[arg5]\n\t"
        "mov r10, %[arg4]\n\t"
        "mov rdx, %[arg3]\n\t"
        "int 0x80" : "=a"(result) : "a"(number),
        [arg1] "D"(arg1), [arg2] "S"(arg2), [arg3] "m"(arg3), [arg4] "m"(arg4), [arg5] "m"(arg5), [arg6] "m"(arg6) : "memory", "rcx", "r11");
    return result;
}

