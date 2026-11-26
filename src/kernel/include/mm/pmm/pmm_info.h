/*
 * File: pmm_info.h
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <boot/params.h>
#include "frame_allocator.h"
#include "asm/mmu_arch.h"

typedef struct pmm_info
{
    paddr_t phys_start;
    paddr_t phys_end;
} pmm_info_t;
