/*
 * File: spinlock.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "kernel/threading/spinlock.h"

typedef struct spinlock
{
    volatile uint32_t lock;
} spinlock_t;

static inline void spinlock_acquire(spinlock_t *s)
{
    while (__sync_lock_test_and_set(&s->lock, 1))
    {
        while (s->lock)
        {
            cpu_relax();
        }
    }
}

static inline void spinlock_release(spinlock_t *s)
{
    __sync_lock_release(&s->lock);
}

