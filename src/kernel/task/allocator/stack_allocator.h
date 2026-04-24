/*
 * File: stack_allocator.h
 * File Created: 18 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "task/process.h"

#include <stdint.h>

#define USER_STACK_TOP      (virt_addr)0xC0000000          // base of your kernel mapping
#define USER_STACK_INITIAL  0x1000              // 1 page (4KB)
#define USER_STACK_MAX      (8 * 1024 * 1024)   // 8MB hard limit

// Grows the stack by one page downward. Called from your #PF handler.
// Returns 1 on success, 0 if stack would exceed USER_STACK_MAX.
int stack_grow(process *proc);

// Frees all mapped stack pages. Called on process exit.
void stack_free(process *proc);

// Allocates and maps the initial user stack into proc's page directory.
// Sets proc->stack_top and proc->stack_size.
// Returns the initial ESP value to load into the process registers.
virt_addr stack_alloc_init(process *proc);
