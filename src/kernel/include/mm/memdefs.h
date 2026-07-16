/*
 * File: memdefs.h
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define MEMORY_MIN (vaddr_t)0x0000000000000000
#define MEMORY_MAX (vaddr_t)0xFFFFFFFFFFFFFFFFull

// ? (32) or 0x0000000040000000 (64)
#define USER_HEAP_SIZE (vaddr_t)0x0000000010000000ull
#define USER_HEAP_BASE (vaddr_t)0x0000000040000000ull

// ? (32) or 0x000000007FFFF000 (64)
#define USER_VDSO_VIRT (vaddr_t)0x000000007FFFF000ull

#define MMAP_ARENA_START   0x0000000080000000ULL   // just above VDSO
#define MMAP_ARENA_END     0x00007FFFE0000000ULL   // just below user stack

// 0x00000000C0000000 (32) or 0x0000800000000000 (64)
#ifdef __i686__
#define USER_STACK_END
#define USER_STACK_TOP (vaddr_t)0x00000000C0000000ull
#else
#define USER_STACK_SIZE (vaddr_t)0x000000001FFFD000ull
#define USER_STACK_END  (vaddr_t)0x00007FFFE0000000ull
#define USER_STACK_TOP  (vaddr_t)0x00007FFFFFFFE000ull
#endif

// ? (32) or 0xFFFF800000000000 (64)
#define MEMORY_DIRECT_MAP_VIRT_BASE (vaddr_t)0xFFFF800000000000ull

// ? (32) or 0xFFFFFFFC00000000 (64)
#define MEMORY_MMIO_VIRT_BASE (vaddr_t)0xFFFFFFFC00000000ull

#define MEMORY_PHYS_COPY_SRC    (vaddr_t)0xFFFFFFFFF0000000ull // scratch window 1
#define MEMORY_PHYS_COPY_DST    (vaddr_t)0xFFFFFFFFF0001000ull // scratch window 2

// ? (32) or 0xFFFFFFFE00000000 (64)
#define MEMORY_STACKS_VIRT_BASE (vaddr_t)0xFFFFFFFE00000000ull

// ? (32) or 0xFFFFFFFF00000000 (64)
#define MEMORY_HEAP_VIRT_BASE (vaddr_t)0xFFFFFFFF00000000ull

// 0xC0000000 (32) or 0xFFFFFFFF80000000 (64)
#define MEMORY_KERNEL_VIRT_BASE KERNEL_VIRT_BASE
