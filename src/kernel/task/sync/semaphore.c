/*
 * File: semaphore.c
 * File Created: 29 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "semaphore.h"

#include "kernel/cpu.h"
#include "task/threading/thread_type.h"
#include "task/threading/scheduling/scheduler.h"
#include "errno/errno.h"

#define MODULE "SEM"

void sem_init(semaphore_t *s, int initial)
{
    s->count = initial;
    list_init(&s->waiters);
}

void sem_wait(semaphore_t *s)
{
    thread_t *self = cpu_arch_get_current()->current;

    lock(&s->lock);
    while (s->count <= 0)
    {
        self->state = THREAD_BLOCKED;
        list_push_tail(&s->waiters, &self->node);
        unlock(&s->lock);
        sched_yield();
        lock(&s->lock);
    }
    s->count--;
    unlock(&s->lock);
}

void sem_post(semaphore_t *s)
{
    lock(&s->lock);
    s->count++;
    list_node_t *n = list_pop_head(&s->waiters);
    unlock(&s->lock);

    if (n == (void *)-EPERM)
    {
        return;
    }

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