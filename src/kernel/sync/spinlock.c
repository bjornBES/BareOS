/*
 * File: spinlock.c
 * File Created: 18 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sync/spinlock.h"

void spinlock_acquire(spinlock_t *lock)
{
    spinlock_arch_acquire(lock);
}

void spinlock_release(spinlock_t *lock)
{
    spinlock_arch_release(lock);
}