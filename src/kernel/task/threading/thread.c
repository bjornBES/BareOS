/*
 * File: thread.c
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "thread.h"
#include "priority.h"
#include "kernel.h"

#include "kernel/threading/threading.h"
#include "kernel/ctx.h"
#include "kernel/ivt.h"
#include "kernel/memory.h"

#include "mm/kstack/kstack_allocator.h"

#include "debug/debug.h"

#define MODULE "THREAD"

static tid_t next_tid = 0;

static inline tid_t get_tid()
{
    return next_tid++;
}

static inline thread_t *thread_allocate()
{
    thread_t *t = malloc(sizeof(thread_t));
    if (!t)
    {
        KERNEL_PANIC(MODULE, "OOM");
    }
    memset(t, 0, sizeof(thread_t));
    return t;
}

void thread_set_priority(thread_t *t, int priority)
{
    t->priority = priority;
    t->timeslice_reset = priority_to_timeslice(priority);
    t->timeslice = t->timeslice_reset;
}

thread_t *thread_create_kernel(uintptr_t (*entry)(void *), void *arg)
{
    log_debug(MODULE, "thread_create(%p)", entry);
    thread_t *t = thread_allocate();
    vaddr_t kstack = kstack_alloc();

    t->kernel_stack_top = kstack;
    t->kernel_stack = thread_arch_ctx_init(t, entry, kstack, (vaddr_t)arg);

    t->tid = get_tid();
    thread_set_priority(t, PRIORITY_NORMAL_SHORT);
    t->state = THREAD_READY;
    t->is_kernel_thread = true;

    return t;
}

thread_t *thread_create_user(process_t *proc, uint64_t ustack)
{
    thread_t *t = thread_allocate();

    vaddr_t kstack = kstack_alloc();

    // fake iretq frame — CPU pops these on iretq
    log_debug(MODULE, "ustack = 0x%lx", ustack);
    t->kernel_stack_top = kstack;
    t->kernel_stack = thread_user_arch_ctx_init(t, proc->entry, kstack, ustack, 0);
    log_debug(MODULE, "t->kernel_stack = 0x%lx", t->kernel_stack);
    t->tid = get_tid();
    thread_set_priority(t, PRIORITY_NORMAL);
    t->is_user_thread = true;
    t->state = THREAD_READY;
    t->proc = proc;

    return t;
}

thread_t *thread_create_user_tid(process_t *proc, uint64_t ustack, tid_t tid)
{
    thread_t *t = thread_allocate();

    vaddr_t kstack = kstack_alloc();

    // fake iretq frame — CPU pops these on iretq
    log_debug(MODULE, "ustack = 0x%lx", ustack);
    t->kernel_stack_top = kstack;
    t->kernel_stack = thread_user_arch_ctx_init(t, proc->entry, kstack, ustack, 0);
    log_debug(MODULE, "t->kernel_stack = 0x%lx", t->kernel_stack);
    t->tid = tid;
    thread_set_priority(t, PRIORITY_NORMAL);
    t->is_user_thread = true;
    t->state = THREAD_READY;
    t->proc = proc;

    return t;
}

thread_t *thread_create_main(vaddr_t entry)
{
    thread_t *t = thread_allocate();

    vaddr_t kstack = kstack_alloc();
    t->kernel_stack_top = kstack;
    t->kernel_stack = ctx_arch_init(&t->ctx, (vaddr_t)entry, kstack, 0, 0);

    t->tid = get_tid();
    thread_set_priority(t, PRIORITY_HIGH);
    t->is_kernel_thread = true;
    t->state = THREAD_READY;

    log_info(MODULE, "got kstack %p", kstack);

    return t;
}

thread_t *thread_create_from(thread_t *parent, intr_frame_t *current_frame, uint64_t ustack)
{
    thread_t *child = thread_allocate();
    if (!child)
    {
        return NULL;
    }

    // copy basic fields
    child->tid = get_tid();
    child->state = THREAD_READY;
    thread_set_priority(child, parent->priority);

    // signal state — copy mask, but pending signals start empty
    child->blocked_signals = parent->blocked_signals;
    // child->pending_signals = 0;
    // memcpy(child->sigactions, parent->sigactions, sizeof(child->sigactions));

    // fresh kernel kstack — never share this
    log_debug(MODULE, "child @ %p", child);
    child->kernel_stack = kstack_alloc();
    child->kernel_stack_top = child->kernel_stack;
    child->is_user_thread = true;
    log_debug(MODULE, "child->kernel_stack @ %p", child->kernel_stack);

    // set up context to resume into the cloned frame
    log_debug(MODULE, "child->ctx @ %p, child->kernel_stack @ %p, child_frame @ %p", &child->ctx, child->kernel_stack, current_frame);
    child->kernel_stack = thread_arch_ctx_init_fork(child, child->kernel_stack, current_frame);

    ivt_dump_frame(current_frame);
    ctx_dump(&child->ctx);

    return child;
}
