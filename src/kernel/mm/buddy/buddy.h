/*
 * File: buddy.h
 * File Created: 16 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

#include <boot/bootparams.h>

#define BUDDY_MAX_ORDER 11 // 2^0 * 4KiB to 2^10 * 4KiB = 4KiB to 4MiB
#define MAX_SIZE (1 << (BUDDY_MAX_ORDER - 1)) * PAGE_SIZE
#define LOW_SIZE 1 * PAGE_SIZE

void buddy_init(memory_entry_t *memory);
void buddy_map();
paddr_t buddy_alloc(uint32_t order);
paddr_t buddy_alloc_at(paddr_t addr);
void buddy_free(paddr_t phys_addr, uint32_t order);
void buddy_print_info();
void buddy_print_info_verbose();
