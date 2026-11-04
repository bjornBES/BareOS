/*
 * File: spinlock.h
 * File Created: 18 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/spinlocks_arch.h"

#include <types.h>
#include <defs.h>

#define SPIN_NAME(name) CONCAT(__lock_, name)

#define DEF_LOCK(name)                                            \
    INTERNAL spinlock_t SPIN_NAME(name) = {0};                    \
    INTERNAL ALWAYS_INLINE void CONCAT(__acquire_lock_, name)()   \
    {                                                             \
        spinlock_acquire(&SPIN_NAME(name));                       \
    }                                                             \
    INTERNAL ALWAYS_INLINE void CONCAT(__acquire_unlock_, name)() \
    {                                                             \
        spinlock_release(&SPIN_NAME(name));                       \
    }
#define SPIN_LOCK(name)               CONCAT(__acquire_lock_, name)();
#define SPIN_UNLOCK(name)             CONCAT(__acquire_unlock_, name)();

#define LOCK()                        CONCAT(__acquire_lock_, LOCK_NAME)();
#define UNLOCK()                      CONCAT(__acquire_unlock_, LOCK_NAME)();

#define SPIN_WARN_LOCK_UNLOCKED(name) WARN_ON(!spinlock_is_locked(&SPIN_NAME(name)))
#define WARN_LOCK_UNLOCKED()          WARN_ON(!spinlock_is_locked(&SPIN_NAME(LOCK_NAME)))

void spinlock_acquire_log(spinlock_t *lock);
void spinlock_release_log(spinlock_t *lock);

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
bool spinlock_is_locked(spinlock_t *lock);
