/*
 * File: paging.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"

#include "frame.h"

#include "debug/debug.h"

#include "libs/malloc.h"
#include "libs/stdio.h"

#include <util/binary.h>

#define MODULE "PAGING"

extern char __end;
extern char __heap_size;

#ifdef __x86_64__
typedef uint64_t uint32_64;
#else
typedef uint32_t uint32_64;
#endif

void paging_init(boot_params *boot_params)
{
    frame_init();

    virt_addr heap_start_virt = (virt_addr)&__end;
    size_t heap_size = (size_t)&__heap_size;
    heap_init(heap_start_virt, heap_size);

    #ifdef __x86_64__
    paging_init64();
    #else
    paging_init32();
#endif
    
    // kernel_page = (page_directory32*)boot_params->pageDirectory;
}

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
phys_addr paging_alloc_frame()
{
    phys_addr i = frame_alloc_frame();
    if (i == 0)
    {
        log_err(MODULE, "Out of pages got %u", i);
        return NULL;
    }
    if (i < KERNEL_PHYS_BASE)
    {
        log_warn("MAIN", "Something may be wrong");
        // frame_dump_bitmap();
    }
    log_debug(MODULE, "got frame index %u", (uint32_64)i / PAGE_SIZE);

    return i;
}

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
phys_addr paging_alloc_frame_at(phys_addr physAddr)
{
    uint32_64 frame_index = (uint32_64)physAddr / PAGE_SIZE;

    if (frame_get_frame(frame_index))
    {
        log_err(MODULE, "Frame %u already claimed", frame_index);
        log_debug(MODULE, "Allocating for %p", physAddr);
        return NULL; // already claimed
    }
    log_debug(MODULE, "got frame index %u", frame_index);

    frame_alloc_at(physAddr);
    return (phys_addr)((uint32_64)physAddr & ~0xFFF); // return page-aligned addr
}

// Free a physical frame by its physical address.
void paging_free_frame(phys_addr physAddr)
{
    uint32_64 frame_index = (uint32_64)physAddr / PAGE_SIZE;

    if (!frame_get_frame(frame_index))
    {
        log_err(MODULE, "Frame %u at %p not allocated", frame_index / PAGE_SIZE, physAddr);
        return;
    }

    frame_mark_frame_free(frame_index);
}

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
phys_addr paging_alloc_and_map(paging_page *page_dir, virt_addr virtAddr, paging_flags flags)
{
    phys_addr phys = paging_alloc_frame();
    if (phys == NULL)
        return NULL; // OOM

    paging_map_page(page_dir, virtAddr, phys, flags);
    return phys;
}

phys_addr paging_alloc_and_map_region(paging_page *page_dir, virt_addr virtAddr, size_t size, paging_flags flags)
{
    virt_addr virt = virtAddr;
    phys_addr org_phys = paging_alloc_frame();
    phys_addr phys = org_phys;

    // Round up to page boundary
    uint32_64 pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_64 i = 0; i < pages; i++)
    {
        paging_map_page(page_dir, virt, phys, flags);
        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
    }
    return org_phys;
}

// Map a contiguous physical region [physAddr, physAddr + size) to
// [virtAddr, virtAddr + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
void paging_map_region(paging_page *page_dir, virt_addr virtAddr, phys_addr physAddr, size_t size, paging_flags flags)
{
    virt_addr virt = virtAddr;
    phys_addr phys = physAddr;

    // Round up to page boundary
    uint32_64 pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_64 i = 0; i < pages; i++)
    {
        paging_map_page(page_dir, virt, phys, flags);
        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
    }
}

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(paging_page *page_dir, virt_addr virtAddr, size_t size)
{
    virt_addr virt = virtAddr;
    // error things maybe?

    // Round up to page boundary
    uint32_64 pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_64 i = 0; i < pages; i++)
    {
        paging_unmap_page(page_dir, (virt_addr)virt);
        virt += PAGE_SIZE;
    }
}