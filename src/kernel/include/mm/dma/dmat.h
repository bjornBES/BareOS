/*
 * File: dmat.h
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

/// @ingroup kernel_dma
/// @{

/// @brief 
/// @phys: 
/// @virt: 
/// @pages: 
/// @size: 
typedef struct dma
{
    paddr_t phys;
    vaddr_t virt;

    // number of PAGE_SIZE blocked
    blkcnt_t pages;

    size_t size;
} dma_t;

/// @}
