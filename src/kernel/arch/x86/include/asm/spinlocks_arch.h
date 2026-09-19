/*
 * File: spinlocks_arch.h
 * File Created: 18 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct spinlock
{
    volatile uint32_t lock;
    volatile uint64_t flags;
} spinlock_t;

void spinlock_arch_acquire(spinlock_t *lock);
void spinlock_arch_release(spinlock_t *lock);