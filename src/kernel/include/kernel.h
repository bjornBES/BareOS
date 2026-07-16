/*
 * File: kernel.h
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "debug/debug.h"
#include <stddef.h>
#include <stdint.h>
#include <types.h>

#define RETURN_ERROR -1
#define RETURN_FAILED 1
#define RETURN_GOOD 0

extern char __KERNEL_VIRT;
extern char __KERNEL_PHYS;
extern char __kernel_end;
extern char __kernel_end_phys;
extern char __heap_size;
extern char __vdso_start;
extern char __vdso_start_phys;

#define KERNEL_VIRT_BASE (vaddr_t)&__KERNEL_VIRT
#define KERNEL_PHYS_BASE (paddr_t)&__KERNEL_PHYS // 1MB

#define KERNEL_VIRT_END (vaddr_t)&__kernel_end
#define KERNEL_PHYS_END (paddr_t)&__kernel_end_phys

#define KERNEL_SIZE (size_t)(KERNEL_PHYS_END - KERNEL_PHYS_BASE)
#define KERNEL_HEAP_SIZE (size_t)&__heap_size

#define KERNEL_HEAP_VIRT_END (vaddr_t)&__kernel_end + KERNEL_HEAP_SIZE
#define KERNEL_HEAP_PHYS_END (paddr_t)&__kernel_end_phys + KERNEL_HEAP_SIZE

#define KERNEL_VDSO_VIRT (vaddr_t)&__vdso_start
#define KERNEL_VDSO_PHYS (paddr_t)&__vdso_start_phys

#define KERNEL_STACK_SIZE (16 * 1024)  // 24k

#define ALIGN_DOWN(x, a) ((x) - ((x) % (a)))
#define ALIGN_UP(x, a) (((x) + (a) - 1) / (a) * (a))
#define ALIGN_2_DOWN(x, a) ((x) & ~((a) - 1))
#define ALIGN_2_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))

void panic(char *module, char *file, int line, char *message, ...);
void kernel_panic(char *message, ...);

