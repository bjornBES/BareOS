/*
 * File: thread.h
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "thread_type.h"
#include "task/process_types.h"
#include "kernel/ctx.h"
#include "mm/allocator/memory_allocator.h"
#include "mm/kstack/kstack_allocator.h"

#define THREAD_EXIT(t)              \
    {                               \
        t->state = THREAD_DEAD;     \
        t->tid = 0;                 \
        kstack_free(t->kernel_stack); \
        kfree(t);                   \
    }
// mark dead and yield — scheduler will skip dead threads
// implemented after scheduler exists

thread_t *thread_create_from_current();
thread_t *thread_create_main();
thread_t *thread_create_from(thread_t *parent, intr_frame_t *current_frame, uint64_t user_stack_top);
thread_t *thread_create(void (*entry)(void));
thread_t *thread_create_user(process_t *proc, uint64_t user_stack_top);
