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

int pmm_reg_get_allocator(pmm_info_t *info, frame_allocator_t table[CONFIG_MAX_FRAME_ALLOCATORS], int *out_index);
int pmm_reg_initialize(pmm_info_t *info, frame_allocator_t table[CONFIG_MAX_FRAME_ALLOCATORS]);
