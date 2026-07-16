/*
 * File: mmu.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "kernel/mmu.h"
#include "kernel/ctx.h"

int mmu_page_fault_handler(intr_frame_t *regs, mmu_fault_info *info);

void mmu_copy_contents(paddr_t src, paddr_t dst);

// Allocate a free frame AND map it at virt in one step.
// Returns the physical address on success, NULL on OOM.
paddr_t mmu_alloc_and_map(page_table_t *table, vaddr_t virt, mmu_flags_t flags);

paddr_t mmu_alloc_and_map_region(page_table_t *table, vaddr_t virt, size_t size, mmu_flags_t flags);

// Map a contiguous physical region [phys, phys + size) to
// [virt, virt + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
paddr_t mmu_map_region(page_table_t *table, vaddr_t virt, paddr_t phys, size_t size, mmu_flags_t flags);

// Unmap [virt, virt + size) and free the backing frames.
void mmu_free_region(page_table_t *table, vaddr_t virt, size_t size);