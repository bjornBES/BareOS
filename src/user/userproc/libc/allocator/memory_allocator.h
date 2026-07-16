/*
 * File: memory_allocator.h
 * File Created: 07 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
	uint8_t status;
	uint32_t size;
} alloc_t;

void allocator_init();
void allocator_print_status();
void allocator_print_blocks();

void* 	impl_malloc(size_t size);
void 	impl_free(void* ptr);
void* 	impl_calloc(size_t num, size_t size);
void* 	impl_realloc(void* ptr, size_t size);
