/*
 * File: pmm.h
 * File Created: 16 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#include <boot/bootparams.h>

#include "kernel.h"

#define HEAP_FRAME 0x4000

extern paddr_t start;
extern paddr_t end;
extern size_t mem_size;

vaddr_t phys_to_virt_auto(paddr_t p);
paddr_t virt_to_phys_auto(vaddr_t p);

/// @brief Allocate a range of physical frames as used.
///
/// @param start_phys Start address of the region (inclusive).
/// @param end_phys End address of the region (exclusive).
paddr_t pmm_alloc_region(paddr_t start_phys, paddr_t end_phys);

/// @brief Allocate a specific physical frame.
///
/// @param addr Physical address of the frame to allocate.
paddr_t pmm_alloc_at(paddr_t addr);

/// @brief Allocate the next free physical frame.
///
/// @return Physical address of the allocated frame or NULL on failure.
paddr_t pmm_alloc_frame();

paddr_t pmm_alloc_frame_at_size(size_t size);

/// @brief Free a previously allocated physical frame.
///
/// @param addr Physical address of the frame to free.
void pmm_free_frame(paddr_t addr);

void pmm_ref_frame(paddr_t phys);
void pmm_deref_frame(paddr_t phys);
uint16_t pmm_get_refcount(paddr_t phys);

paddr_t pmm_alloc_heap_frame();

void pmm_print_info();
void pmm_print_info_verbose();

paddr_t pmm_init(boot_params_t *params);

void pmm_map();