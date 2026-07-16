/*
 * File: spinlock_types.h
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct spinlock
{
    volatile uint32_t lock;
} spinlock_t;