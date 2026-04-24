/*
 * File: pmm.h
 * File Created: 16 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#include <boot/bootparams.h>

#include "kernel.h"
#include "buddy.h"

#define HEAP_FRAME 4096*4

extern phys_addr start;
extern phys_addr end;

virt_addr phys_to_virt_auto(phys_addr p);

/// @brief Allocate a range of physical frames as used.
///
/// @param start_phys Start address of the region (inclusive).
/// @param end_phys End address of the region (exclusive).
phys_addr pmm_alloc_region(phys_addr start_phys, phys_addr end_phys);

/// @brief Allocate a specific physical frame.
///
/// @param phys_addr Physical address of the frame to allocate.
phys_addr pmm_alloc_at(phys_addr addr);

/// @brief Allocate the next free physical frame.
///
/// @return Physical address of the allocated frame or NULL on failure.
phys_addr pmm_alloc_frame();

/// @brief Free a previously allocated physical frame.
///
/// @param phys_addr Physical address of the frame to free.
void pmm_free_frame(phys_addr addr);

phys_addr pmm_alloc_heap_frame();

void pmm_print_info();
void pmm_print_info_verbose();

void pmm_init(boot_params *params);

void pmm_map();