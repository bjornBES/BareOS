/*
 * File: thread.c
 * File Created: 19 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "thread/thread.h"
#include "thread/threadt.h"

#include "asm/thread_arch.h"

#include "mm/allocator/kstack_allocator.h"

#include "kerrno.h"
#include "memory.h"
#include "panic.h"

#define MODULE "thread"

static tid_t next_tid = 0;

static INTERNAL ALWAYS_INLINE tid_t get_tid()
{
    return next_tid++;
}

INTERNAL INLINE thread_t *thread_allocate()
{
    thread_t *t = kmalloc(sizeof(thread_t));
    if (t == NULL)
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

thread_t *thread_create_kernel(thread_func_t entry, uintptr_t arg, uint32_t flags)
{
    ENTER_FUNC("%p, %p, 0x%x", entry, arg, flags);
    thread_t *t = thread_allocate();
    vaddr_t kstack = kstack_alloc(&t->kernel_stack);

    t->kstack = thread_arch_ctx_init(t, entry, kstack, arg);

    t->tid = get_tid();
    t->state = THREAD_READY;
    thread_set_priority(t, PRIORITY_NORMAL_SHORT);
    return t;
}

thread_t *thread_create(thread_func_t entry, uintptr_t arg, uint32_t flags)
{
    ENTER_FUNC("%p, %p, 0x%x", entry, arg, flags);
    thread_t *t = thread_allocate();
    vaddr_t kstack = kstack_alloc(&t->kernel_stack);

    t->kstack = thread_arch_ctx_init(t, entry, kstack, arg);

    t->tid = get_tid();
    t->state = THREAD_READY;
    thread_set_priority(t, PRIORITY_NORMAL_SHORT);
    return t;
}
