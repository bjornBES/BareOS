/*
 * File: signal.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "signal.h"
#include "kernel/signal/signal.h"
#include "task/threading/thread_type.h"
#include "task/process_types.h"
#include "kernel/x86.h"
#include "kernel/ivt.h"
#include "kernel/memory.h"
#include "kernel/segments.h"
#include "kernel/asm/cpuid/cpuid.h"
#include "mm/memdefs.h"

#include <util/binary.h>
#include <defs.h>

#define MODULE "X86-SIGNAL"

/* void signal_arch_return(syscall_info *info)
{
    intr_frame_t *regs = info->regs;
    signal_frame *frame = (signal_frame *)regs->sp;
    memcpy(regs, &frame->proc_reg, sizeof(intr_frame_t));
} */

uint64_t frame_uc_flags(intr_frame_t *frame)
{
    uint64_t flags;

    cpuid_regs_t regs;
    cpuid(1, 0, &regs);

    if (BIT_GET(regs.ecx, 27) == 1)
    {
        flags = UC_FP_XSTATE | UC_SIGCONTEXT_SS;
    }
    else
    {
        flags = UC_SIGCONTEXT_SS;
    }

    if (frame->cs == (USER_CODE_SELECTOR | 0x3))
    {
        flags |= UC_STRICT_RESTORE_SS;
    }

    return flags;
}

void signal_arch_setup_frame(thread_t *t, intr_frame_t *frame, siginfo_t *info, sigaction_t *sa)
{
    process_t *proc = t->proc;
    vaddr_t user_stack = frame->sp;
    if (FLAG_IS_SET(sa->sa_flags, SA_ONSTACK))
    {
        // TODO add SA_ONSTACK in year 2030
    }

    user_stack -= sizeof(signal_arch_frame_t);
    user_stack &= ~0xFull;

    signal_arch_frame_t *sf = (signal_arch_frame_t *)user_stack;

    sigset_t saved_mask = t->blocked_signals;
    sf->uc.uc_sigmask = saved_mask;
    sf->pretcode = (void *)USER_VDSO_VIRT;
    sf->uc.uc_flags = frame_uc_flags(frame);
    sf->uc.uc_stack.ss_sp = (void *)frame->sp;
    sf->uc.uc_stack.ss_size = proc->vma->start_stack - proc->vma->stack_vma->top;
    
    sigcontext_t *context = &sf->uc.uc_mcontext;
    context->di = frame->di;
    context->si = frame->si;
    context->bp = frame->bp;
    context->sp = frame->sp;
    context->bx = frame->bx;
    context->dx = frame->dx;
    context->ax = frame->ax;
    context->cx = frame->cx;
    #ifdef __x86_64__
    context->r8 = frame->r8;
    context->r9 = frame->r9;
    context->r10 = frame->r10;
    context->r11 = frame->r11;
    context->r12 = frame->r12;
    context->r13 = frame->r13;
    context->r14 = frame->r14;
    context->r15 = frame->r15;
    #else
    #endif
    context->trapno = frame->interrupt;
    context->err = frame->error;
    context->pc = frame->pc;
    context->flags = frame->flags;
    context->cs = frame->cs;
    context->gs = 0;
    context->fs = 0;
    context->ss = frame->ss;
    
    context->fpstate = NULL;
    
    context->oldmask = sa->sa_mask;
    vaddr_t cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));
    context->cr2 = cr2;
    
    
    t->blocked_signals |= sa->sa_mask;
    if (FLAG_IS_SET(sa->sa_flags, SA_NODEFER) == 0)
    {
        t->blocked_signals |= SIGMASK(info->si_signo);
    }

    log_debug(MODULE, "frame @ %p", frame);
    log_debug(MODULE, "sf @ %p", sf);
    log_debug(MODULE, "context @ %p", context);

    frame->pc = (vaddr_t)sa->handler.sa_handler;
    frame->sp = user_stack;
    frame->di = info->si_signo;
    frame->cs = USER_CODE_SELECTOR | 3;
    frame->ss = USER_DATA_SELECTOR | 3;
    frame->ds = USER_DATA_SELECTOR | 3;
}

void signal_arch_restore_frame(intr_frame_t *frame)
{
    signal_arch_frame_t *sf = (signal_arch_frame_t *)(frame->sp & ~0xFull);
    sigcontext_t *context = &sf->uc.uc_mcontext;

    log_debug(MODULE, "sf @ %p", sf);
    log_debug(MODULE, "context @ %p", context);

    frame->di = context->di;
    frame->si = context->si;
    frame->bp = context->bp;
    frame->sp = context->sp;
    frame->bx = context->bx;
    frame->dx = context->dx;
    frame->ax = context->ax;
    frame->cx = context->cx;
#ifdef __x86_64__
    frame->r8 = context->r8;
    frame->r9 = context->r9;
    frame->r10 = context->r10;
    frame->r11 = context->r11;
    frame->r12 = context->r12;
    frame->r13 = context->r13;
    frame->r14 = context->r14;
    frame->r15 = context->r15;
#else
#endif
    frame->pc = context->pc;
    frame->flags = context->flags;
    frame->cs = USER_CODE_SELECTOR | 3;
    frame->ss = USER_DATA_SELECTOR | 3;
    frame->ds = USER_DATA_SELECTOR | 3;
}

void signal_arch_dispatch(intr_frame_t *frame)
{
    ctx_arch_switch((vaddr_t)frame);
}

void signal_arch_setup_vdso()
{
}
