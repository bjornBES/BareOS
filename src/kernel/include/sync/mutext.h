/*
 * File: mutex.h
 * File Created: 08 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "spinlock.h"
#include "lists/list_node.h"
#include "thread/threadt.h"

#include <types.h>

typedef struct
{
    spinlock_t lock; // protects the fields below, held only briefly
    bool locked;
    thread_t *owner; // for debugging / detecting recursive-lock bugs
    list_t waiters;
} mutex_t;