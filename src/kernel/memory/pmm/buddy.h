/*
 * File: buddy.h
 * File Created: 16 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

#include <boot/bootparams.h>

#define BUDDY_MAX_ORDER 11 // 2^0 * 4KiB to 2^10 * 4KiB = 4KiB to 4MiB


void buddy_init(boot_params *params);
void buddy_map();
phys_addr buddy_alloc(uint32_t order);
phys_addr buddy_alloc_at(phys_addr addr);
void buddy_free(phys_addr phys_addr, uint32_t order);
void buddy_print_info();
void buddy_print_info_verbose();
