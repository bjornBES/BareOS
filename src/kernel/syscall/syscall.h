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
    union {
        struct {
			uint32_t esi;
            uint32_t edi;
			uint32_t edx;
			uint32_t ecx;
			uint32_t ebx;
			uint32_t eax;
		};
		struct {
			uint16_t si, hsi;
            uint16_t di, hdi;
			uint16_t cx, hcx;
			uint16_t dx, hdx;
			uint16_t bx, hbx;
			uint16_t ax, hax;
		} U16;
		struct {
            uint8_t sil, sih, esi2, esi3;
            uint8_t dil, dih, edi2, edi3;
			uint8_t dl, dh, edx2, edx3;
			uint8_t cl, ch, ecx2, ecx3;
			uint8_t bl, bh, ebx2, ebx3;
			uint8_t al, ah, eax2, eax3;
		} U8;
	};
} syscall_registers;
typedef int (*syscall_handler)(syscall_registers *regs);

#define SYSCALL_DEFINE0(func, ret)             \
    ret sys_##func##(syscall_registers * regs) \
    {                                          \
        return func();                         \
    }

#define SYSCALL_DEFINE1(func, ret, t1)         \
    ret sys_##func##(syscall_registers * regs) \
    {                                          \
        t1 arg1 = (t1)(uintptr_t)(regs->edi);  \
        return func(arg1);                     \
    }

#define SYSCALL_DEFINE2(func, ret, t1, t2)     \
    ret sys_##func##(syscall_registers * regs) \
    {                                          \
        t1 arg1 = (t1)(uintptr_t)(regs->edi);  \
        t2 arg2 = (t2)(uintptr_t)(regs->esi);  \
        return func(arg1, arg2);               \
    }

#define SYSCALL_DEFINE3(func, ret, t1, t2, t3) \
    ret sys_##func(syscall_registers *regs)    \
    {                                          \
        t1 arg1 = (t1)(uintptr_t)(regs->edi);  \
        t2 arg2 = (t2)(uintptr_t)(regs->esi);  \
        t3 arg3 = (t3)(uintptr_t)(regs->edx);  \
        return func(arg1, arg2, arg3);         \
    }

#define SYSCALL_ENTRY(number, name) [number] = sys_##name

void syscall_init();
