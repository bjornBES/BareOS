/*
 * File: stack_allocator.h
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "mm/memdefs.h"

#define KERNEL_STACK_SLOT   (KERNEL_STACK_SIZE + PAGE_SIZE)  // +guard page

void kstack_init();
vaddr_t kstack_per_cpu_alloc();
vaddr_t kstack_alloc();
void kstack_free(vaddr_t stack_top);
