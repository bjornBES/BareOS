/*
 * File: sched_class.h
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "thread/threadt.h"

#include <types.h>

typedef struct sched_algorithm_ops
{
    void *(*init)(void *runq_data);                             // set up whatever internal structure this algo needs
    thread_t *(*get_head)(void *runq_data, int level);
    int (*thread_count)(void *runq_data);
    status_t (*enqueue)(void *runq_data, thread_t *t);              // add a runnable thread
    thread_t *(*pick_next)(void *runq_data);                    // choose next thread to run, remove from queue
    status_t (*tick)(void *runq_data, thread_t *current);           // called every scheduler tick — algo decides slice/demote/etc
    status_t (*yield)(void *runq_data, thread_t *current);          // called on voluntary yield/block — algo decides promote/etc
    bool (*should_preempt)(void *runq_data, thread_t *current); // can a higher-priority thread jump the queue?
    thread_t *(*steal_one)(void *runq_data);                    // add to the vtable
} sched_algorithm_ops_t;

typedef struct sched_class
{
    char *name;
    int max_level;
    sched_algorithm_ops_t ops;
} sched_class_t;