/*
 * File: memory.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "mm/allocator/memory_allocator.h"

#include "kernel/asm/memory.h"

static inline void copy_to_user(void *dst, void *src, size_t size)
{
    memcpy(dst, src, size);
}
static inline void copy_from_user(void *dst, void *src, size_t size)
{
    memcpy(dst, src, size);
}

void *kmalloc_phys(size_t size, void **virt_out);
void *kcalloc_phys(size_t num, size_t size, void **virt_out);
