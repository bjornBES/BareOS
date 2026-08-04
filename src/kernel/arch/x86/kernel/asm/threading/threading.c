/*
 * File: threading.c
 * File Created: 02 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/threading/threading.h"
#include "task/threading/scheduling/scheduler.h"

#include "kernel/x86.h"
#include "kernel/ivt.h"
#include "kernel/segments.h"
#include "kernel/memory.h"

#define MODULE "X86-THREAD"

vaddr_t thread_arch_ctx_init(thread_t *t, uintptr_t (*entry)(void *), vaddr_t kstack, uint64_t arg)
{
    ENTER_FUNC(MODULE, "%p, %p, %p, %p, %p", t, entry, kstack, arg);
    // carve a fake frame and stack at top of kernel stack
    
    vaddr_t stack_frame = kstack & ~0xFull;
    stack_frame -= sizeof(uint64_t);
    *((vaddr_t *)stack_frame) = (uint64_t)(void*)entry;
    log_debug(MODULE, "entry = %p", *((vaddr_t *)stack_frame));
    
    stack_frame -= sizeof(uint64_t);
    *((uint64_t *)stack_frame) = arg;
    
    // vaddr_t args = stack_frame;
    
    // log_debug(MODULE, "stack_frame = %p", stack_frame);
    stack_frame -= sizeof(intr_frame_t);
    intr_frame_t *frame = (intr_frame_t *)stack_frame;
    memset(frame, 0, sizeof(intr_frame_t));

    frame->sp = stack_frame;
    frame->cs = KERNEL_CODE_SELECTOR;
    frame->ss = KERNEL_DATA_SELECTOR;
    frame->ds = KERNEL_DATA_SELECTOR;
    frame->pc = (vaddr_t)kthread_entry;

    frame->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;
    frame->di = (uint64_t)(void*)entry;
    frame->si = arg;

    // point ctx at the fake frame
    t->ctx.frame.sp = kstack;
    t->ctx.frame.regs = frame;
    
    // ivt_dump_frame(frame);
    return stack_frame;
}

vaddr_t thread_user_arch_ctx_init(thread_t *t, vaddr_t entry, vaddr_t kstack, vaddr_t ustack, uint64_t arg)
{
    ENTER_FUNC(MODULE, "%p, %p, %p, %p, %p", t, entry, kstack, ustack, arg);
    // carve a fake frame at top of kernel stack
    kstack -= sizeof(intr_frame_t);
    intr_frame_t *frame = (intr_frame_t *)kstack;
    memset(frame, 0, sizeof(intr_frame_t));

    if (ustack == 0)
    {
    }
    else
    {
        frame->sp = ustack;
        frame->cs = USER_CODE_SELECTOR | 3;
        frame->ss = USER_DATA_SELECTOR | 3;
        frame->ds = USER_DATA_SELECTOR | 3;
        frame->pc = (vaddr_t)entry;
    }
    frame->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;
    frame->di = arg;

    // point ctx at the fake frame
    t->ctx.frame.sp = kstack;
    t->ctx.frame.regs = frame;
    return kstack;
}

vaddr_t thread_arch_ctx_init_fork(thread_t *t, vaddr_t kstack, intr_frame_t *parent_frame)
{
    kstack -= sizeof(intr_frame_t);
    intr_frame_t *frame = (intr_frame_t *)kstack;
    ctx_arch_clone_frame(frame, parent_frame);

    // frame->sp = (reg_t)child_frame;
    frame->cs = USER_CODE_SELECTOR | 3;
    frame->ss = USER_DATA_SELECTOR | 3;
    frame->ds = USER_DATA_SELECTOR | 3;
    frame->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;

    // point ctx at the fake frame
    t->ctx.frame.sp = kstack;
    t->ctx.frame.regs = frame;
    ctx_arch_set_ip(&t->ctx, parent_frame->pc);
    return kstack;
}

void kthread_entry(uintptr_t (*entry)(void *), void *arg)
{
    uintptr_t ret = entry(arg);
    sched_thread_exit(ret);
}

