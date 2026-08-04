/*
 * File: mutex.h
 * File Created: 29 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "task/threading/spinlock/spinlock.h"
#include "task/threading/thread_type.h"
#include "lists/list.h"
#include <types.h>

typedef struct
{
    spinlock_t lock; // protects the fields below, held only briefly
    bool locked;
    thread_t *owner; // for debugging / detecting recursive-lock bugs
    list_t waiters;
} mutex_t;

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);
