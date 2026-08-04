/*
 * File: spinlock.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct spinlock
{
    volatile uint32_t lock;
} spinlock_t;

extern void spinlock_arch_lock(volatile uint32_t *lock);
extern void spinlock_arch_unlock(volatile uint32_t *lock);
void cpu_relax();