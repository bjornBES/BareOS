/*
 * File: semaphore.h
 * File Created: 29 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "task/threading/spinlock/spinlock.h"
#include "task/threading/scheduling/sched_types.h"
#include "lists/list.h"
#include <types.h>

typedef struct
{
    spinlock_t lock;
    int count;
    block_queue_t waiters;
    list_t queue_list;
} semaphore_t;

void sem_init(semaphore_t *s, int initial);

void sem_wait(semaphore_t *s);

void sem_post(semaphore_t *s);
