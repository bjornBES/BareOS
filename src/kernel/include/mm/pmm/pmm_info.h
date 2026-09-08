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

typedef struct pmm_info
{
    memory_entry_t *memory;
    paddr_t phys_start;
    paddr_t phys_end;
    
    vaddr_t virt_start;
    
    size_t mem_size;
    frame_allocator_t *allocator;

    paddr_t kernel_phys_base;
    paddr_t kernel_phys_end;

    vaddr_t kernel_virt_base;
    vaddr_t kernel_virt_end;
} pmm_info_t;
