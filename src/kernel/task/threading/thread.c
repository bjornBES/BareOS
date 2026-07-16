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

thread_t *thread_create(void (*entry)())
{
    log_debug(MODULE, "thread_create(%p)", entry);
    thread_t *new_thread = malloc(sizeof(thread_t));
    vaddr_t stack = kstack_alloc();

    new_thread->kernel_stack = ctx_arch_init(&new_thread->ctx, (vaddr_t)entry, stack, 0, 0);

    new_thread->tid = get_tid();
    new_thread->priority = PRIORITY_HIGH;
    new_thread->timeslice_reset = priority_to_timeslice(PRIORITY_HIGH);
    new_thread->timeslice = new_thread->timeslice_reset;
    new_thread->state = THREAD_READY;

    return new_thread;
}

thread_t *thread_create_user(process_t *proc, uint64_t user_stack_top)
{
    thread_t *t = malloc(sizeof(thread_t));

    vaddr_t kstack = kstack_alloc();

    // fake iretq frame — CPU pops these on iretq
    log_debug(MODULE, "user_stack_top = 0x%lx", user_stack_top);
    t->kernel_stack = ctx_arch_init(&t->ctx, proc->entry, kstack, user_stack_top, 0);
    log_debug(MODULE, "t->kernel_stack = 0x%lx", t->kernel_stack);
    t->tid = get_tid();
    t->priority = PRIORITY_NORMAL;
    t->timeslice_reset = priority_to_timeslice(PRIORITY_NORMAL);
    t->timeslice = t->timeslice_reset;
    t->state = THREAD_READY;
    t->proc = proc;

    return t;
}

thread_t *thread_create_main()
{
    thread_t *t = malloc(sizeof(thread_t));
    memset(t, 0, sizeof(thread_t));
    vaddr_t stack_top = kstack_alloc();
    t->kernel_stack = stack_top;
    t->tid = get_tid();
    t->timeslice_reset = 20;
    t->timeslice = 20;
    t->state = THREAD_READY;

    log_info(MODULE, "got stack %p", stack_top);

    return t;
}

thread_t *thread_create_from(thread_t *parent, intr_frame_t *current_frame, uint64_t user_stack_top)
{
    thread_t *child = kmalloc(sizeof(thread_t));
    if (!child)
    {
        return NULL;
    }

    // copy basic fields
    child->tid = get_tid();
    child->state = THREAD_READY;
    child->priority = parent->priority;
    child->timeslice_reset = 20;
    child->timeslice = 20;

    // signal state — copy mask, but pending signals start empty
    child->signal_mask = parent->signal_mask;
    // child->pending_signals = 0;
    // memcpy(child->sigactions, parent->sigactions, sizeof(child->sigactions));

    // fresh kernel stack — never share this
    log_debug(MODULE, "child @ %p", child);
    child->kernel_stack = kstack_alloc();
    log_debug(MODULE, "child->kernel_stack @ %p", child->kernel_stack);

    // clone the interrupt frame — arch owned
    intr_frame_t *parent_frame = current_frame; // saved by ISR
    intr_frame_t *child_frame = (intr_frame_t *)(child->kernel_stack - sizeof(intr_frame_t));

    log_debug(MODULE, "child_frame @ %p, parent_frame @ %p", child_frame, parent_frame);
    ctx_arch_clone_frame(child_frame, parent_frame);

    // set child's return value to 0 (fork convention)
    frame_arch_set_retval(child_frame, 0);

    // set up context to resume into the cloned frame
    log_debug(MODULE, "child->ctx @ %p, child->kernel_stack @ %p, child_frame @ %p", &child->ctx, child->kernel_stack, child_frame);
    child->kernel_stack = ctx_arch_init_fork(&child->ctx, child->kernel_stack, child_frame);

    ivt_dump_frame(current_frame);
    ctx_dump(&child->ctx);

    vaddr_t pc = frame_arch_get_ip(current_frame);
    ctx_arch_set_ip(&child->ctx, pc);
    // ctx_arch_set_sp(&child->ctx, user_stack_top);

    return child;
}

