/*
 * File: kstack_allocator.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#include "stack_info/stack_info.h"

#define KERNEL_STACK_SIZE    (16 * 1024) // 24k
#define KERNEL_STACK_SLOT   (KERNEL_STACK_SIZE + PAGE_SIZE)  // +guard page

status_t kstack_alloc(stack_info_t *info, vaddr_t *out_addr);
status_t kstack_per_cpu_alloc(stack_info_t *info, vaddr_t *out_addr);
void kstack_free(vaddr_t stack_top);