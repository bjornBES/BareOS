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
#include "frame.h"
#include "task/process.h"
#include "debug/debug.h"
#include "libs/memory.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <core/Defs.h>
#include <util/binary.h>
#include <boot/bootparams.h>

#ifdef __i686__
#include "paging_32.h"
#else
#include "paging_64.h"
#endif

extern bool paging_print_out;

typedef void* phys_addr;
typedef void* virt_addr;

extern char __KERNEL_VIRT;
extern char __KERNEL_PHYS;

#define KERNEL_VIRT_BASE (void *)&__KERNEL_VIRT
#define KERNEL_PHYS_BASE (void *)&__KERNEL_PHYS // 1MB

#define PAGE_SIZE 4096

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_PWT (1 << 3)
#define PAGE_PCD (1 << 4)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_DIRTY (1 << 6)
#define PAGE_HUGE_PAGES (1 << 7)
#define PDE_GLOBAL (1 << 7)

typedef struct paging_page_t
{
    void *page_dir;
} paging_page;

extern paging_page *kernel_page;

#define paging_native_flush_tlb_single(addr) __asm__ volatile("invlpg %0" ::"m" (addr) : "memory");

void paging_init32();
void paging_init64();

virt_addr phys_to_virt(phys_addr phys);

void paging_create_user_directory(void *_proc);

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
phys_addr paging_get_physical(paging_page *page_dir, virt_addr virtAddr);

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
virt_addr paging_get_virtual(paging_page *page_dir, phys_addr physAddr);

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
phys_addr paging_alloc_frame();

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
phys_addr paging_alloc_frame_at(phys_addr physAddr);

// Free a physical frame by its physical address.
void paging_free_frame(phys_addr physAddr);

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(paging_page *page_dir, virt_addr virtAddr, phys_addr physAddr, paging_flags flags);

// Unmap a single virtual page (does NOT free the underlying frame).
void paging_unmap_page(paging_page *page_dir, virt_addr virtAddr);

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
phys_addr paging_alloc_and_map(paging_page *page_dir, virt_addr virtAddr, paging_flags flags);

phys_addr paging_alloc_and_map_region(paging_page *page_dir, virt_addr virtAddr, size_t size, paging_flags flags);

// Map a contiguous physical region [physAddr, physAddr + size) to
// [virtAddr, virtAddr + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
void paging_map_region(paging_page *page_dir, virt_addr virtAddr, phys_addr physAddr, size_t size, paging_flags flags);

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(paging_page *page_dir, virt_addr virtAddr, size_t size);

void paging_print_info(virt_addr cr2);

void paging_init(boot_params *boot_params);