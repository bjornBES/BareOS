/*
 * File: threading.c
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "threading.h"

#include "kernel/gdt.h"
#include "kernel/x86.h"

registers *thread_create_regs(void *stack_pointer, void *entry)
{
    registers *regs = (registers *)stack_pointer;

    // fake iretq frame — CPU pops these on iretq
    regs->ss = x86_KERNEL_DATA_SEGMENT;          // ss
    regs->sp = (uint64_t)regs; // rsp
    regs->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;
    regs->cs = x86_KERNEL_CODE_SEGMENT; // cs
    regs->pc = (uint64_t)entry;         // rip

    regs->error = 0;     // error
    regs->interrupt = 0; // interrupt

    regs->ds = x86_KERNEL_DATA_SEGMENT; // ds

#ifdef __x86_64__
    regs->r15 = 0; // r15
    regs->r14 = 0; // r14
    regs->r13 = 0; // r13
    regs->r12 = 0; // r12
    regs->r11 = 0; // r11
    regs->r10 = 0; // r10
    regs->r9 = 0;  // r9
    regs->r8 = 0;  // r8
#endif
    regs->di = 0; // rdi
    regs->si = 0; // rsi
    regs->bp = 0; // rbp
    regs->bx = 0; // rbx
    regs->dx = 0; // rdx
    regs->cx = 0; // rcx
    regs->ax = 0; // rax
    return regs;
}

registers *thread_create_user_regs(void *sp, void *entry, uint64_t user_rsp)
{
    registers *regs = (registers *)sp;

    regs->ss = x86_USER_DATA_SEGMENT | 3; // 0x23
    regs->sp = user_rsp;              // user stack
    regs->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;
    regs->cs = x86_USER_CODE_SEGMENT | 3; // 0x1B
    regs->pc = (uint64_t)entry;       // user entry point

    regs->error = 0;
    regs->interrupt = 0;

    regs->ds = x86_USER_DATA_SEGMENT | 3;

#ifdef __x86_64__
    regs->r15 = 0; // r15
    regs->r14 = 0; // r14
    regs->r13 = 0; // r13
    regs->r12 = 0; // r12
    regs->r11 = 0; // r11
    regs->r10 = 0; // r10
    regs->r9 = 0;  // r9
    regs->r8 = 0;  // r8
#endif
    regs->di = 0; // rdi
    regs->si = 0; // rsi
    regs->bp = 0; // rbp
    regs->bx = 0; // rbx
    regs->dx = 0; // rdx
    regs->cx = 0; // rcx
    regs->ax = 0; // rax

    return regs;
}
registers *thread_create_regs_from(void *dest_sp, registers *src_regs)
{
    registers *regs = (registers *)dest_sp;

    memcpy(regs, src_regs, sizeof(registers));

    return regs;
}

void *thread_get_function(void *stack_pointer)
{
    registers *regs = (registers *)stack_pointer;
    return (void*)regs->pc;
}