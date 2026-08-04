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
#include "priority.h"

#define THREAD_EXIT(t)                    \
    {                                     \
        t->state = THREAD_DEAD;           \
        t->tid = 0;                       \
        kstack_free(t->kernel_stack_top); \
        kfree(t);                         \
    }
// mark dead and yield — scheduler will skip dead threads
// implemented after scheduler exists

#define GET_THREAD(list)                                   \
    list_node_t *node = list_pop_head(&(list));            \
    thread_t *thread = container_of(node, thread_t, node);

thread_t *thread_create_kernel(uintptr_t (*entry)(void *), void *arg);
thread_t *thread_create_from_current();
thread_t *thread_create_main(vaddr_t entry);
thread_t *thread_create_from(thread_t *parent, intr_frame_t *current_frame, uint64_t user_stack_top);
thread_t *thread_create_user(process_t *proc, uint64_t user_stack_top);
thread_t *thread_create_user_tid(process_t *proc, uint64_t user_stack_top, tid_t tid);
void thread_set_priority(thread_t *t, int priority);