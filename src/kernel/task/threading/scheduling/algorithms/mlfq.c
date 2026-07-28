/*
 * File: mlfq.c
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mlfq.h"
#include "kernel/config.h"
#include "kernel/memory.h"
#include "task/threading/priority.h"

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
            return container_of(n, thread_t, node);
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

void mlfq_yield(void *runq_data, thread_t *current)
{
    (void)runq_data;
    if (current->priority + MLFQ_PROMOTE_STEP <= PRIORITY_HIGH)
    {
        current->priority = current->priority + MLFQ_PROMOTE_STEP;
    }
    else
    {
        current->priority = PRIORITY_HIGH;
    }
    current->timeslice_reset = priority_to_timeslice(current->priority);
}

void mlfq_tick(void *runq_data, thread_t *current)
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
            current->priority = PRIORITY_LOW;
        }
        current->timeslice_reset = priority_to_timeslice(current->priority);
    }
}

thread_t *mlfq_pick_next(void *runq_data)
{
    mlfq_data_t *q = runq_data;
    log_debug(MODULE, "q = %p", q);
    if (q->count == 0)
    {
        return NULL;
    }
    for (int b = MLFQ_NUM_BUCKETS - 1; b >= 0; b--)
    {
        log_debug(MODULE, "&q->queues[%u] = %p", b, &q->queues[b]);
        if (q->queues[b].count > 0)
        {
            list_node_t *n = list_pop_head(&q->queues[b]);
            if (n)
            {
                q->count--;
                thread_t *t = container_of(n, thread_t, node);
                t->timeslice = t->timeslice_reset; // reload cached value, no recompute
                return t;
            }
        }
    }
    return NULL;
}

void mlfq_enqueue(void *runq_data, thread_t *t)
{
    mlfq_data_t *q = runq_data;
    int b = priority_to_bucket(t->priority);
    list_push_tail(&q->queues[b], &t->node);
    q->count++;
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
        runq_data = malloc(sizeof(mlfq_data_t));
        q = runq_data;
    }
    for (int i = 0; i < MLFQ_NUM_BUCKETS - 1; i++)
    {
        list_init(&q->queues[i]);
    }
    q->count = 0;
    return q;
}

sched_class_t mlfq_class;

const sched_class_t *mlfq_get_class()
{
    mlfq_class.name = "MLFQ";
    mlfq_class.ops.init = mlfq_init;
    mlfq_class.ops.thread_count = mlfq_thread_count;
    mlfq_class.ops.enqueue = mlfq_enqueue;
    mlfq_class.ops.pick_next = mlfq_pick_next;
    mlfq_class.ops.tick = mlfq_tick;
    mlfq_class.ops.yield = mlfq_yield;
    mlfq_class.ops.should_preempt = mlfq_should_preempt;
    mlfq_class.ops.steal_one = mlfq_steal_one;
    return &mlfq_class;
}

#endif
