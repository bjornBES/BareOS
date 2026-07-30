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

#include "task/threading/spinlock/spinlock_types.h"
#include "lists/list.h"
#include <types.h>

typedef struct
{
    spinlock_t lock;
    int count;
    list_t waiters;
} semaphore_t;

void sem_init(semaphore_t *s, int initial);

void sem_wait(semaphore_t *s);

void sem_post(semaphore_t *s);
