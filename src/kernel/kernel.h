/*
 * File: main.h
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#define RETURN_ERROR -1
#define RETURN_FAILED 0
#define RETURN_GOOD 1

extern char __KERNEL_VIRT;
extern char __KERNEL_PHYS;
extern char __kernel_end;
extern char __kernel_end_phys;
extern char __heap_size;

typedef void *phys_addr;
typedef void *virt_addr;

#define KERNEL_VIRT_BASE (virt_addr)&__KERNEL_VIRT
#define KERNEL_PHYS_BASE (phys_addr)&__KERNEL_PHYS // 1MB

#define KERNEL_VIRT_END (virt_addr)&__kernel_end
#define KERNEL_PHYS_END (phys_addr)&__kernel_end_phys

#define KERNEL_SIZE (size_t)(KERNEL_PHYS_END - KERNEL_PHYS_BASE)
#define KERNEL_HEAP_SIZE (size_t)&__heap_size

#define KERNEL_HEAP_VIRT_END (virt_addr)&__kernel_end + KERNEL_HEAP_SIZE
#define KERNEL_HEAP_PHYS_END (phys_addr)&__kernel_end_phys + KERNEL_HEAP_SIZE

#define PAGE_SIZE 4096

#define ALIGN_DOWN(x, a) ((x) - ((x) % (a)))
#define ALIGN_UP(x, a) (((x) + (a) - 1) / (a) * (a))
#define ALIGN_2_DOWN(x, a) ((x) & ~((a) - 1))
#define ALIGN_2_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))

#ifdef __x86_64__
typedef uint64_t uint32_64;
#else
typedef uint32_t uint32_64;
#endif

void panic(char *module, char *file, int line, char *message, ...);
void kernel_panic(char *message, ...);
