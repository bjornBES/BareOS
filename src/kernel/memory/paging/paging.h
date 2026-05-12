/*
 * File: paging.h
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "paging_type.h"

#include "debug/debug.h"
#include "kernel.h"
#include "memory/allocator/memory_allocator.h"
#include "memory/flags.h"

#include "kernel/paging/paging.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <defs.h>
#include <util/binary.h>
#include <boot/bootparams.h>

#define PAGE_SIZE 4096

extern bool paging_print_out;
extern paging_page_t kernel_page;

void reload_pages();
void page_copy(phys_addr src, phys_addr dst);

uint64_t mm_flags_to_pte(memory_flags_t flags);
memory_flags_t pte_to_mm_flags(uint64_t pte);

void paging_get_current_cr3(paging_page_t *paging_phys);
void paging_load_cr3(paging_page_t new_paging_page);

virt_addr paging_phys_to_virt(phys_addr phys);
phys_addr paging_virt_to_phys(virt_addr virt);

phys_addr paging_get_kernel_frame();

void paging_create_user_directory(void *_proc);
void paging_free_user_directory(void *_proc);
void paging_copy_kernel_mappings(void *_proc);

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
phys_addr paging_get_physical(paging_page_t paging_page_t, virt_addr virtAddr);

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
virt_addr paging_get_virtual(paging_page_t paging_page_t, phys_addr physAddr);

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
phys_addr paging_alloc_frame();

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
phys_addr paging_alloc_frame_at(phys_addr physAddr);

// Allocate the specific physical region of frames containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
phys_addr paging_alloc_frame_region(phys_addr physAddr, size_t size);

// Free a physical frame by its physical address.
void paging_free_frame(phys_addr physAddr);

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(paging_page_t paging_page_t, virt_addr virtAddr, phys_addr physAddr, memory_flags_t flags);

// Unmap a single virtual page (does NOT free the underlying frame).
void paging_unmap_page(paging_page_t paging_page_t, virt_addr virtAddr);

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
phys_addr paging_alloc_and_map(paging_page_t paging_page_t, virt_addr virtAddr, memory_flags_t flags);

phys_addr paging_alloc_and_map_region(paging_page_t paging_page_t, virt_addr virtAddr, size_t size, memory_flags_t flags);

// Map a contiguous physical region [physAddr, physAddr + size) to
// [virtAddr, virtAddr + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
void paging_map_region(paging_page_t paging_page_t, virt_addr virtAddr, phys_addr physAddr, size_t size, memory_flags_t flags);

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(paging_page_t paging_page_t, virt_addr virtAddr, size_t size);

void paging_print_info(paging_page_t page_dir, virt_addr cr2);

void paging_init(boot_params *boot_params);