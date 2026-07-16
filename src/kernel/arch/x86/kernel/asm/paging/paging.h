/*
 * File: x86_paging.h
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#ifdef __i686__
#include "paging_32.h"
#else
#include "paging_64.h"
#endif

#include "debug/debug.h"
#include "kernel/mmu_types.h"
#include "kernel/memory.h"
#include "kernel.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <defs.h>
#include <util/binary.h>
#include <boot/bootparams.h>

#define paging_native_flush_tlb_single(addr)                            \
    __asm__ volatile("invlpg [%0]" ::"r"((uintptr_t)(addr)) : "memory")

uint64_t mm_flags_to_pte(mmu_flags_t flags)
{
    uint64_t pte = 0;

    if (flags.present)
    {
        FLAG_SET(pte, PAGE_PRESENT);
    }
    if (flags.write)
    {
        FLAG_SET(pte, PAGE_WRITABLE);
    }
    if (flags.user)
    {
        FLAG_SET(pte, PAGE_USER);
    }
    if (!flags.exec)
    {
        FLAG_SET(pte, PAGE_NO_EXEC);
    }
    if (flags.cached)
    {
        FLAG_SET(pte, PAGE_PCD | PAGE_PWT);
    }
    if (flags.global)
    {
        FLAG_SET(pte, PAGE_GLOBAL);
    }
    if (flags.cow)
    {
        FLAG_SET(pte, PAGE_COW);
    }

    return pte;
}

mmu_flags_t pte_to_mm_flags(uint64_t pte)
{
    mmu_flags_t f = {0};
    f.present = FLAG_IS_SET(pte, PAGE_PRESENT);
    f.write = FLAG_IS_SET(pte, PAGE_WRITABLE);
    f.user = FLAG_IS_SET(pte, PAGE_USER);
    f.exec = !FLAG_IS_SET(pte, PAGE_NO_EXEC);
    f.cached = FLAG_IS_SET(pte, PAGE_PCD);
    f.global = FLAG_IS_SET(pte, PAGE_GLOBAL);
    f.cow = FLAG_IS_SET(pte, PAGE_COW);
    return f;
}

void paging_clean_up(page_table_t *page_table, vaddr_t virtAddr);
void page_copy(paddr_t src, paddr_t dst);

uint64_t mm_flags_to_pte(mmu_flags_t flags);
mmu_flags_t pte_to_mm_flags(uint64_t pte);

vaddr_t paging_phys_to_virt(paddr_t phys);
paddr_t paging_virt_to_phys(vaddr_t virt);

paddr_t paging_get_kernel_frame();

void paging_create_user_directory(page_table_t *result);
void paging_free_user_directory(void *_proc);
void paging_copy_kernel_mappings(void *_proc);

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
paddr_t paging_get_physical(page_table_t *paging_page_t, vaddr_t virtAddr);

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
vaddr_t paging_get_virtual(page_table_t *paging_page_t, paddr_t physAddr);

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
paddr_t paging_alloc_frame();

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
paddr_t paging_alloc_frame_at(paddr_t physAddr);

// Allocate the specific physical region of frames containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
paddr_t paging_alloc_frame_region(paddr_t physAddr, size_t size);

// Free a physical frame by its physical address.
void paging_free_frame(paddr_t physAddr);

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
int paging_map_page(page_table_t *paging_page_t, vaddr_t virtAddr, paddr_t physAddr, mmu_flags_t flags);

// Unmap a single virtual page (does NOT free the underlying frame).
paddr_t paging_unmap_page(page_table_t *paging_page_t, vaddr_t virtAddr);

paddr_t paging_print_info(page_table_t *page_dir, vaddr_t cr2);

void paging_init(boot_params_t *boot_params);

void arch_paging_init(boot_params_t *boot_params, paddr_t pmm_start, paddr_t pmm_end);
