/*
 * File: pmm_register.h
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "mm/pmm/pmm_info.h"
#include "frame_allocator.h"
#include <types.h>
#include <config.h>

/// @brief 
/// @param info 
/// @param table 
/// @param out_index 
/// @return KERRNO_SUCCESSES on successes or an kerrno number
/// @retval KERRNO_BAD_INDEX: if ALLOCATOR_COUNT is over or equals to CONFIG_MAX_FRAME_ALLOCATORS
/// @retval KERRNO_NO_INIT: if allocator setup failed
status_t pmm_reg_get_allocator(pmm_info_t *info, frame_allocator_t table[CONFIG_MAX_FRAME_ALLOCATORS], int *out_index);

/// @brief 
/// @param info 
/// @param table 
/// @return KERRNO_SUCCESSES on successes or an kerrno number
/// @retval KERRNO_BAD_INDEX: if ALLOCATOR_COUNT is over or equals to CONFIG_MAX_FRAME_ALLOCATORS
/// @retval KERRNO_NO_INIT: if allocator initialize failed
status_t pmm_reg_initialize(pmm_info_t *info, frame_allocator_t table[CONFIG_MAX_FRAME_ALLOCATORS]);
