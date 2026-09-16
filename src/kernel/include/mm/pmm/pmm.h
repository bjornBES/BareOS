/*
 * File: pmm.h
 * File Created: 01 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "pmm_info.h"

#include <boot/params.h>
#include <types.h>

status_t pmm_early_init(boot_params_t *bp);
status_t pmm_init();

/// @brief Allocate the next free physical frame.
///
/// @return Physical address of the allocated frame or kerrno code on failure.
paddr_t pmm_alloc_frame();

/// @brief Free a previously allocated physical frame.
///
/// @param addr Physical address of the frame to free.
/// @return KERRNO_SUCCESSES or kerrno code on failure.
status_t pmm_free_frame(paddr_t addr);

/// @brief Allocate a specific physical frame.
///
/// @param addr Physical address of the frame to allocate.
/// @return Physical address of the allocated frame or kerrno code on failure.
paddr_t pmm_alloc_at(paddr_t addr);

/// @brief Allocate a range of physical frames as used.
///
/// @param start_phys Start address of the region (inclusive).
/// @param end_phys End address of the region (exclusive).
/// @return Physical address of the first allocated frame or kerrno code on failure.
paddr_t pmm_alloc_region(paddr_t start_phys, paddr_t end_phys);

/// @brief 
/// @param size 
/// @return Physical address of the first allocated frame or kerrno code on failure.
paddr_t pmm_alloc_frame_at_size(size_t size);

/// @brief 
/// @param times 
/// @return Physical address of the first allocated frame or kerrno code on failure.
paddr_t pmm_alloc_frames_contiguous(size_t times);

status_t pmm_ref_frame(paddr_t phys);
status_t pmm_deref_frame(paddr_t phys);
uint16_t pmm_get_refcount(paddr_t phys);

pmm_info_t *pmm_get_info();

void pmm_print_info();
void pmm_print_info_verbose();

vaddr_t phys_to_virt_auto(paddr_t p);
paddr_t virt_to_phys_auto(vaddr_t p);