/*
 * File: thread.c
 * File Created: 21 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/thread_arch.h"
#include "asm/contextt_arch.h"
#include "asm/frame_arch.h"

#include "sched/sched.h"

#include "entry/desc/gdt/segments.h"
#include "kernel/cpu/cpu_flags.h"

#include "debug/debug.h"

#include "memory.h"

#include <config.h>

#define MODULE "x86-thread"

void kthread_entry(thread_func_t entry, uintptr_t arg)
{
    uintptr_t ret = entry(arg);
    sched_thread_exit(ret);
}

status_t thread_arch_ctx_init(thread_t *thread, thread_func_t entry, vaddr_t kstack, uintptr_t arg)
{
    ENTER_FUNC(MODULE, "%p, %p, %p", thread, entry, arg);
    // carve a fake frame and stack at top of kernel stack

    vaddr_t stack_frame = kstack & ~0xFull;
    stack_frame -= sizeof(uint64_t);
    *((vaddr_t *)stack_frame) = (uint64_t)(void *)entry;
    trace_debug(MODULE, "entry = %p", *((vaddr_t *)stack_frame));

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
    frame->es = KERNEL_DATA_SELECTOR;
#if CONFIG_ENABLE_SMP == 0
    frame->gs = KERNEL_DATA_SELECTOR;
    frame->fs = KERNEL_DATA_SELECTOR;
#endif
    frame->pc = (vaddr_t)kthread_entry;

    frame->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;
    frame->di = (uint64_t)(void *)entry;
    frame->si = arg;

    // frame_arch_dump_frame(frame);

    // point ctx at the fake frame
    thread->ctx.regs = frame;

    // ivt_dump_frame(frame);
    return stack_frame;
}
