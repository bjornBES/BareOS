/*
 * File: mlfq.c
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sched/mlfq/mlfq.h"
#include "memory.h"
#include "thread/priority.h"

#include "kerrno.h"

#include "memory.h"

#include <types.h>
#include <config.h>

#if CONFIG_INCLUDE_MLFQ == 1

#define MLFQ_NUM_BUCKETS  4
#define MLFQ_DEMOTE_STEP  32 // how much priority drops on full-slice-used
#define MLFQ_PROMOTE_STEP 32 // how much priority rises on early yield

#define MODULE "MLFQ"

typedef struct
{
    list_t queues[MLFQ_NUM_BUCKETS]; // bucket 0 = lowest priority range, 3 = highest
    int count;
} mlfq_data_t;

static inline int priority_to_bucket(uint8_t priority)
{
    return (priority * MLFQ_NUM_BUCKETS) / 256; // 0-63->0, 64-127->1, 128-191->2, 192-255->3
}

thread_t *mlfq_steal_one(void *runq_data)
{
    mlfq_data_t *q = runq_data;
    // steal from the lowest-priority non-empty bucket first — least disruptive to give away
    for (int b = 0; b < MLFQ_NUM_BUCKETS; b++)
    {
        list_node_t *n = list_pop_tail(&q->queues[b]);
        if (n)
        {
            q->count--;
            thread_t *t = container_of(n, thread_t, node);
            t->in_queue = false;
            return t;
        }
    }
    return NULL;
}

bool mlfq_should_preempt(void *runq_data, thread_t *current)
{
    mlfq_data_t *q = runq_data;
    int my_bucket = priority_to_bucket(current->priority);
    for (int b = MLFQ_NUM_BUCKETS - 1; b > my_bucket; b--)
    {
        if (!list_is_empty(&q->queues[b]))
        {
            return true; // something strictly higher priority is waiting
        }
    }
    return false;
}

status_t mlfq_yield(void *runq_data, thread_t *current)
{
    if (current == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "current thread is null");
    }
    (void)runq_data;
    if (current->priority + MLFQ_PROMOTE_STEP <= PRIORITY_L8)
    {
        current->priority = current->priority + MLFQ_PROMOTE_STEP;
    }
    else
    {
        current->priority = PRIORITY_L8;
    }
    current->timeslice_reset = priority_to_timeslice(current->priority);

    return KERRNO_SUCCESSES;
}

status_t mlfq_tick(void *runq_data, thread_t *current)
{
    (void)runq_data;
    if (current->timeslice > 0)
    {
        current->timeslice--;
    }

    if (current->timeslice == 0)
    {
        if (current->priority > MLFQ_DEMOTE_STEP)
        {
            current->priority = current->priority - MLFQ_DEMOTE_STEP;
        }
        else
        {
            current->priority = PRIORITY_L0;
        }
        current->timeslice_reset = priority_to_timeslice(current->priority);
    }
    return KERRNO_SUCCESSES;
}

thread_t *mlfq_pick_next(void *runq_data)
{
    mlfq_data_t *q = runq_data;
    if (q->count == 0)
    {
        return NULL;
    }
    for (int b = MLFQ_NUM_BUCKETS - 1; b >= 0; b--)
    {
        // log_debug(MODULE, "&q->queues[%u] = %p", b, &q->queues[b]);
        if (q->queues[b].count > 0)
        {
            list_node_t *n = list_pop_head(&q->queues[b]);
            if (n)
            {
                q->count--;
                thread_t *t = container_of(n, thread_t, node);
                t->in_queue = false;
                t->timeslice = t->timeslice_reset; // reload cached value, no recompute
                return t;
            }
        }
    }
    return NULL;
}

status_t mlfq_enqueue(void *runq_data, thread_t *t)
{
    if (t->in_queue)
    {
        KERRNO_RETURN(KERRNO_PERMISSION_DENIED, "thread is already in a queue");
    }
    
    t->in_queue = true;
    mlfq_data_t *q = runq_data;
    int b = priority_to_bucket(t->priority);
    list_push_tail(&q->queues[b], &t->node);
    q->count++;
    return KERRNO_SUCCESSES;
}

thread_t *mlfq_get_first(void *runq_data, int level)
{
    mlfq_data_t *q = runq_data;
    thread_t *t = container_of(q->queues[level].head, thread_t, node);
    return t;
}

int mlfq_thread_count(void *runq_data)
{
    mlfq_data_t *q = runq_data;
    return q->count;
}

void *mlfq_init(void *runq_data)
{
    mlfq_data_t *q = runq_data;
    if (runq_data == NULL)
    {
        runq_data = kmalloc(sizeof(mlfq_data_t));
        q = runq_data;
    }
    for (int i = 0; i < MLFQ_NUM_BUCKETS - 1; i++)
    {
        list_init(&q->queues[i]);
    }
    q->count = 0;
    return q;
}

status_t mlfq_setup(sched_class_t *class)
{
    class->name = "MLFQ";
    class->max_level = MLFQ_NUM_BUCKETS - 1;
    class->ops.init = mlfq_init;
    class->ops.get_head = mlfq_get_first;
    class->ops.thread_count = mlfq_thread_count;
    class->ops.enqueue = mlfq_enqueue;
    class->ops.pick_next = mlfq_pick_next;
    class->ops.tick = mlfq_tick;
    class->ops.yield = mlfq_yield;
    class->ops.should_preempt = mlfq_should_preempt;
    class->ops.steal_one = mlfq_steal_one;
    return KERRNO_SUCCESSES;
}

#endif
