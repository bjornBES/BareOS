/*
 * File: threading.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "task/threading/thread_type.h"

vaddr_t thread_arch_ctx_init(thread_t *t, uintptr_t (*entry)(void *), vaddr_t kstack, uint64_t arg);
vaddr_t thread_user_arch_ctx_init(thread_t *t, vaddr_t entry, vaddr_t kstack, vaddr_t ustack, uint64_t arg);
vaddr_t thread_arch_ctx_init_fork(thread_t *t, vaddr_t kstack, intr_frame_t *parent_frame);

void kthread_entry(uintptr_t (*entry)(void *), void *arg);