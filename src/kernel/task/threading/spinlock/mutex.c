/*
 * File: mutex.c
 * File Created: 29 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mutex.h"

#include "kernel/cpu.h"
#include "task/threading/scheduling/scheduler.h"

void mutex_init(mutex_t *m)
{
    m->locked = false;
    m->owner = NULL;
    list_init(&m->waiters);
}

void mutex_lock(mutex_t *m)
{
    thread_t *self = cpu_arch_get_current()->current;

    lock(&m->lock);
    while (m->locked)
    {
        // block ourselves onto this mutex's wait list, atomically w.r.t. m->lock
        self->state = THREAD_BLOCKED;
        list_push_tail(&m->waiters, &self->node);
        unlock(&m->lock);

        sched_yield();  // won't be picked again until sched_wake_one hits our node

        lock(&m->lock); // re-acquire to recheck the condition (avoid lost wakeups)
    }
    m->locked = true;
    m->owner = self;
    unlock(&m->lock);
}

void mutex_unlock(mutex_t *m)
{
    lock(&m->lock);
    m->locked = false;
    m->owner = NULL;

    list_node_t *n = list_pop_head(&m->waiters);
    unlock(&m->lock);

    if (n)
    {
        thread_t *t = container_of(n, thread_t, node);
        if (t->state == THREAD_REMAINS)
        {
            sched_remove(t);
            return;
        }
        t->state = THREAD_READY;
        sched_add(t);
    }
}
