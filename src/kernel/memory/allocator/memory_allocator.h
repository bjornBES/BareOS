/*
 * File: memory_allocator.h
 * File Created: 07 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "memory/paging/paging.h"

typedef struct {
	uint8_t status;
	uint32_t size;
} alloc_t;

void allocator_init();
void allocator_print_status();
void allocator_print_blocks();

void* kmalloc(size_t size);
void kfree(void* ptr);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);
