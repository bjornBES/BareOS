/*
 * File: memory_allocator.h
 * File Created: 07 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <types.h>

typedef struct {
	uint8_t status;
	uint32_t size;
} alloc_t;

void allocator_print_status();
void allocator_print_blocks();

