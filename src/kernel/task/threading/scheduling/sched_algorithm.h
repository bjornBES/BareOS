/*
 * File: sched_algorithm.h
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "task/threading/thread_type.h"
#include "lists/list.h"
#include "kernel.h"

typedef struct sched_algorithm_ops
{
    void *(*init)(void *runq_data);                             // set up whatever internal structure this algo needs
    int (*thread_count)(void *runq_data);
    void (*enqueue)(void *runq_data, thread_t *t);              // add a runnable thread
    thread_t *(*pick_next)(void *runq_data);                    // choose next thread to run, remove from queue
    void (*tick)(void *runq_data, thread_t *current);           // called every scheduler tick — algo decides slice/demote/etc
    void (*yield)(void *runq_data, thread_t *current);          // called on voluntary yield/block — algo decides promote/etc
    bool (*should_preempt)(void *runq_data, thread_t *current); // can a higher-priority thread jump the queue?
    thread_t *(*steal_one)(void *runq_data);                    // add to the vtable
} sched_algorithm_ops_t;

typedef struct sched_class
{
    char *name;
    sched_algorithm_ops_t ops;
} sched_class_t;
