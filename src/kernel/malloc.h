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

typedef struct {
	uint8_t status;
	uint32_t size;
} alloc_t;

void *kmalloc_phys(size_t size, void **virt_out);
void *kcalloc_phys(size_t num, size_t size, void **virt_out);

void heap_init(uint32_t heap_start_virt, size_t heap_size);
void mmInit();
void mmPrintStatus();
void mmPrintBlocks();

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);
