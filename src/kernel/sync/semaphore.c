/*
 * File: semaphore.c
 * File Created: 13 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sync/semaphore.h"

#include "lists/list.h"

#include "sched/sched.h"

#include "asm/cpu_arch.h"

#include "kerrno.h"

#define MODULE "sem"

void sem_init(semaphore_t *s, int initial)
{
    ENTER_FUNC("%p, %u", s, initial);
    #if CONFIG_SEMAPHORE == 1
    s->count = initial;
    list_initialize(&s->queue_list);
    list_initialize(&s->waiters.queue);
    #endif
}

void sem_wait(semaphore_t *s)
{
    ENTER_FUNC("%p", s);
    #if CONFIG_SEMAPHORE == 1
    thread_t *self = THISCPU()->current;

    spinlock_acquire(&s->lock);
    while (s->count <= 0)
    {
        list_push_tail(&s->queue_list, &self->node);
        self->state = THREAD_BLOCKED;
        spinlock_release(&s->lock);
        sched_yield();
        spinlock_acquire(&s->lock);
    }
    s->count--;
    spinlock_release(&s->lock);
    #endif
}

void sem_post(semaphore_t *s)
{
    #if CONFIG_SEMAPHORE == 1
    spinlock_acquire(&s->lock);
    s->count++;
    list_node_t *n = list_pop_head(&s->queue_list);
    spinlock_release(&s->lock);

    if (n == (void *)-EPERM)
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
        log_debug(MODULE, "added %u back in", t->tid);
    }
    #endif
}