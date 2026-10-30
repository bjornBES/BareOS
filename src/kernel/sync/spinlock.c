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

#include "debug/debug.h"

#define MODULE "spinlock"

void spinlock_acquire_log(spinlock_t *lock)
{
    // ENTER_FUNC("%p = { 0x%x, 0x%x }", lock, lock->flags, lock->lock);
    spinlock_arch_acquire(lock);
}

void spinlock_release_log(spinlock_t *lock)
{
    // ENTER_FUNC("%p = { 0x%x, 0x%x }", lock, lock->flags, lock->lock);
    spinlock_arch_release(lock);
}

void spinlock_acquire(spinlock_t *lock)
{
    spinlock_arch_acquire(lock);
}

void spinlock_release(spinlock_t *lock)
{
    spinlock_arch_release(lock);
}