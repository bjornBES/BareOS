/*
 * File: spinlock.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/threading/spinlock/spinlock.h"
#include <types.h>
#include "kernel/irq.h"

#define lock_debug(module, s)                  \
    log_debug(module, "lock spinlock %s", #s); \
    spinlock_acquire(&s);

#define unlock_debug(module, s)                  \
    log_debug(module, "unlock spinlock %s", #s); \
    spinlock_release(&s);

#define lock(module, s)   \
    spinlock_acquire(&s);

#define unlock(module, s) \
    spinlock_release(&s);

static inline void spinlock_acquire(spinlock_t *s)
{
    spinlock_arch_lock(&s->lock);
}

static inline void spinlock_release(spinlock_t *s)
{
    spinlock_arch_unlock(&s->lock);
}

static inline void spinlock_acquire_irq(spinlock_t *s)
{
    irq_arch_disable();
    spinlock_arch_lock(&s->lock);
}

static inline void spinlock_release_irq(spinlock_t *s)
{
    spinlock_arch_unlock(&s->lock);
    irq_arch_enable();
}

