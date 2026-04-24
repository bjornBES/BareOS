/*
 * File: malloc.h
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

void *kmalloc_phys(size_t size, void **virt_out);
void *kcalloc_phys(size_t num, size_t size, void **virt_out);

void heap_init();
void mmInit();
void allocator_print_status();
void allocator_print_blocks();

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);
