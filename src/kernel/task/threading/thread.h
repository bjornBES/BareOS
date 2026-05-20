/*
 * File: thread.h
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "thread_type.h"
#include "task/process_types.h"
#include "kernel/threading/context.h"
#include "memory/allocator/memory_allocator.h"
#include "memory/kstack/kstack_allocator.h"

#define THREAD_EXIT(t)              \
    {                               \
        t->state = THREAD_DEAD;     \
        t->tid = 0;                 \
        kstack_free(t->stack_base); \
        kfree(t);                   \
    }
// mark dead and yield — scheduler will skip dead threads
// implemented after scheduler exists

thread_t *thread_create_from_current();
thread_t *thread_create_main();
thread_t *thread_create_from(thread_t *thr);
thread_t *thread_create(void (*entry)(void));
thread_t *thread_create_user(process_t *proc, uint64_t user_stack_top);
