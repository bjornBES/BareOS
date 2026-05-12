/*
 * File: stack_allocator.h
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "memory/memdefs.h"

#define KERNEL_STACK_SLOT   (KERNEL_STACK_SIZE + PAGE_SIZE)  // +guard page

void kstack_init();
void *kstack_alloc();
void kstack_free(void *stack_top);
