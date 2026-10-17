/*
 * File: memory.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kerrno.h"

#include <memory_lib.h>
#include <defs.h>

status_t kfree(void* ptr);
void* kmalloc(size_t size);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);

void* kzmalloc(size_t size);

void *kmalloc_phys(size_t size, void **virt_out);
void *kcalloc_phys(size_t num, size_t size, void **virt_out);