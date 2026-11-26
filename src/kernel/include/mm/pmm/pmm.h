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

#include "init.h"

#include <boot/params.h>
#include <types.h>

#define FRAME_COUNT(n) n / PAGE_SIZE

status_t pmm_early_init(boot_params_t *bp);

status_t pmm_insert_resource(boot_params_t *params);

/// @brief Allocate the next free physical frame.
///
/// @param[out] out_addr Physical address of the first allocated frame
/// @return KERRNO_SUCCESS on successes or an kerrno number.
status_t pmm_alloc_frame(paddr_t *out_addr);

/// @brief Free a previously allocated physical frame.
///
/// @param addr Physical address of the frame to free.
/// @return KERRNO_SUCCESS or kerrno code on failure.
status_t pmm_free_frame(paddr_t addr);

/// @brief Allocate a specific physical frame.
///
/// @param addr Physical address of the frame to allocate.
/// @param[out] out_addr Physical address of the first allocated frame
/// @return KERRNO_SUCCESS on successes or an kerrno number.
status_t pmm_alloc_at(paddr_t addr, paddr_t *out_addr);

/// @brief Allocate a range of physical frames as used.
///
/// @param start_phys Start address of the region (inclusive).
/// @param end_phys End address of the region (exclusive).
/// @param[out] out_addr Physical address of the first allocated frame
/// @return KERRNO_SUCCESS on successes or an kerrno number.
status_t pmm_alloc_region(paddr_t start_phys, paddr_t end_phys, paddr_t *out_addr);

/// @brief
/// @param[in] nframes
/// @param[out] out_addr Physical address of the first allocated frame
/// @return KERRNO_SUCCESS on successes or an kerrno number.
status_t pmm_alloc_frame_at_size(size_t nframes, paddr_t *out_addr);

/// @brief
/// @param times
/// @param[out] out_addr Physical address of the first allocated frame
/// @return KERRNO_SUCCESS on successes or an kerrno number.
status_t pmm_alloc_frames_contiguous(size_t times, paddr_t *out_addr);

status_t pmm_ref_frame(paddr_t phys);
status_t pmm_deref_frame(paddr_t phys);
uint16_t pmm_get_refcount(paddr_t phys);

pmm_zone_info_t *pmm_get_kernel_zone_info();
pmm_info_t *pmm_get_info();

void pmm_print_info();
void pmm_print_info_verbose();

vaddr_t phys_to_virt_auto(paddr_t p);
paddr_t virt_to_phys_auto(vaddr_t p);

status_t pmm_register_allocator(frame_allocator_t *allocator);

#define INITCALL_ALLOCATOR(__allocator)        \
    static status_t __init __allocator##_init() \
    {                                        \
        return pmm_register_allocator(&(__allocator));      \
    }                                        \
    ALLOCATOR_INITCALL(__allocator##_init)
