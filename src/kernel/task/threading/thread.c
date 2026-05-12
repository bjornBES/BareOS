/*
 * File: thread.c
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "thread.h"
#include "priority.h"

#include "memory/allocator/memory_allocator.h"
#include "memory/kstack/kstack_allocator.h"

#include "kernel.h"

#include "kernel/threading/threading.h"
#include "debug/debug.h"

#define MODULE "THREAD"

static uint32_t next_tid = 0;

thread *thread_create(void (*entry)())
{
    log_debug(MODULE, "thread_create(%p)", entry);
    thread *new_thread = kmalloc(sizeof(thread));
    void *stack = kstack_alloc();

    uint8_t *sp = (uint8_t *)stack - sizeof(registers);

    // fake iretq frame — CPU pops these on iretq
    thread_create_regs(sp, entry);

    new_thread->ctx.stack_pointer = (reg_t)sp;
    new_thread->stack_base = stack;
    new_thread->tid = next_tid++;
    new_thread->priority = PRIORITY_HIGH;
    new_thread->timeslice_reset = priority_to_timeslice(PRIORITY_HIGH);
    new_thread->timeslice = new_thread->timeslice_reset;
    new_thread->state = THREAD_READY;

    log_info(MODULE, "thread%u entry @ %p", new_thread->tid, thread_get_function(sp));

    return new_thread;
}

thread *thread_create_user(process_t *proc, uint64_t user_stack_top)
{
    thread *t = kmalloc(sizeof(thread));

    void *kstack = kstack_alloc();

    uint8_t *sp = (uint8_t *)kstack - sizeof(registers);

    // fake iretq frame — CPU pops these on iretq
    thread_create_user_regs(sp, (void *)proc->entry, user_stack_top);

    t->ctx.stack_pointer = (uint64_t)sp;
    t->stack_base = kstack;
    t->tid = next_tid++;
    t->priority = PRIORITY_NORMAL;
    t->timeslice_reset = priority_to_timeslice(PRIORITY_NORMAL);
    t->timeslice = t->timeslice_reset;
    t->state = THREAD_READY;
    t->proc = proc;

    return t;
}

thread *thread_create_main()
{
    thread *t = kmalloc(sizeof(thread));
    void *stack_top = kstack_alloc();
    t->stack_base = stack_top;
    t->tid = next_tid++;
    t->timeslice_reset = 20;
    t->timeslice = 20;
    t->state = THREAD_READY;
    t->ctx.stack_pointer = 0; // will be filled by context_switch when preempted

    log_info(MODULE, "got stack %p", stack_top);

    return t;
}

thread *thread_create_from(thread *thr)
{
    thread *t = kmalloc(sizeof(thread));
    void *stack_top = kstack_alloc();

    memcpy(t, thr, sizeof(thread));
    t->stack_base = stack_top;
    t->tid = next_tid++;
    t->state = THREAD_READY;
    t->timeslice_reset = priority_to_timeslice(thr->priority);
    t->timeslice = t->timeslice_reset;

    reg_t sp = (reg_t)((uint8_t*)stack_top - sizeof(registers));
    registers *other_regs = (registers *)thr->ctx.stack_pointer;
    registers *this_regs = thread_create_regs_from((void*)sp, other_regs);
    
    t->ctx.stack_pointer = sp;

    log_info(MODULE, "got stack %p", stack_top);

    return t;
}

