/*
 * File: memdefs.h
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

#define MEMORY_MIN (virt_addr)0x0000000000000000
#define MEMORY_MAX (virt_addr)0xFFFFFFFFFFFFFFFFULL

// 0xC0000000 (32) or 0xFFFFFFFF80000000 (64)
#define MEMORY_KERNEL_VIRT_BASE KERNEL_VIRT_BASE

#define MEMORY_PHYS_COPY_SRC  (virt_addr)0xFFFFFFFFF0000000ULL  // scratch window 1
#define MEMORY_PHYS_COPY_DST  (virt_addr)0xFFFFFFFFF0001000ULL  // scratch window 2

// ? (32) or 0xFFFFFFFF00000000 (64)
#define MEMORY_HEAP_VIRT_BASE (virt_addr)0xFFFFFFFF00000000ULL

// ? (32) or 0xFFFFFFFE00000000 (64)
#define MEMORY_STACKS_VIRT_BASE (virt_addr)0xFFFFFFFE00000000ULL

// ? (32) or 0xFFFFFFFC00000000 (64)
#define MEMORY_MMIO_VIRT_BASE (virt_addr)0xFFFFFFFC00000000ULL

// ? (32) or 0xFFFF800000000000 (64)
#define MEMORY_DIRECT_MAP_VIRT_BASE (virt_addr)0xFFFF800000000000ULL

// ? (32) or 0x000000007FFFF000 (64)
#define USER_VDSO_VIRT (virt_addr)0x000000007FFFF000ULL

// ? (32) or 0x0000000040000000 (64)
#define USER_HEAP_BASE (virt_addr)0x0000000040000000ULL

// 0x00000000C0000000 (32) or 0x0000800000000000 (64)
#ifdef __i686__
#define USER_STACK_TOP (virt_addr)0x00000000C0000000ULL
#else
#define USER_STACK_TOP (virt_addr)0x0000800000000000ULL
#endif