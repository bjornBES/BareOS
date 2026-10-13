/*
 * File: mutex.c
 * File Created: 13 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sync/mutex.h"
#include "sync/spinlock.h"

#include "sched/sched.h"

#include "asm/cpu_arch.h"

#include "kerrno.h"

#define MODULE "mutex"

void mutex_init(mutex_t *m)
{
    #if CONFIG_MUTEX == 1
    m->locked = false;
    m->owner = NULL;
    list_initialize(&m->waiters);
    #endif
}

void mutex_lock(mutex_t *m)
{
    #if CONFIG_MUTEX == 1
    // ENTER_FUNC(MODULE, "%p", m);
    thread_t *self = THISCPU()->current;

    spinlock_acquire(&m->lock);
    while (m->locked)
    {
        // block ourselves onto this mutex's wait list, atomically w.r.t. m->lock
        self->state = THREAD_BLOCKED;
        list_push_tail(&m->waiters, &self->node);
        spinlock_release(&m->lock);

        sched_yield();  // won't be picked again until sched_wake_one hits our node

        spinlock_acquire(&m->lock); // re-acquire to recheck the condition (avoid lost wakeups)
    }
    m->locked = true;
    m->owner = self;
    spinlock_release(&m->lock);
    #endif
}

void mutex_unlock(mutex_t *m)
{
    #if CONFIG_MUTEX == 1
    spinlock_acquire(&m->lock);
    m->locked = false;
    m->owner = NULL;
    
    list_node_t *n = list_pop_head(&m->waiters);
    spinlock_release(&m->lock);
    
    if (n == (void *)-KERRNO_NOT_ALLOWED)
    {
        return;
    }

    if (n)
    {
        thread_t *t = container_of(n, thread_t, node);
        if (t->state == THREAD_REMAINS)
        {
            sched_thread_reap(t);
            return;
        }
        t->state = THREAD_READY;
        sched_add(t);
    }
    #endif
}