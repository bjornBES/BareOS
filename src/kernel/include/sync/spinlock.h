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

#include <types.h>

#include "asm/spinlocks_arch.h"

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);