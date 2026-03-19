/*
 * File: paging.c
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"
#include "stdio.h"
#include "debug/debug.h"
#include "frame.h"

#include <memory.h>
#include <string.h>
#include <util/binary.h>

#define PAGE_SIZE 4096

#define MODULE "PAGING"

page_directory_entry *kernel_page_directory = 0;
uint32_t page_dir_loc = 0;

uint32_t *last_page = 0;

void *paging_get_physical(page_directory_entry *page_dir, void *virtualaddr)
{
    uint32_t pdi = GETPAGEDIRECTORYINDEX(virtualaddr);
    uint32_t pti = GETPAGETABLEINDEX(virtualaddr);
    uint32_t offset = (uint32_t)virtualaddr & 0xFFF;

    void *pd_void = (void *)page_dir;
    uint32_t *pd = (uint32_t *)pd_void;
    if (!(pd[pdi] & 0x1))
    {
        log_debug("Paging", "page directory was not pressent with virt %p", virtualaddr);
        return NULL;
    }

    uint32_t *pt = (uint32_t *)(pd[pdi] & ~0xFFF);

    if (!(pt[pdi] & 0x1))
    {
        log_debug("Paging", "page table was not pressent with virt %p", virtualaddr);
        return NULL;
    }

    uint32_t phys_base = pt[pti] & ~0xFFF;
    return (void *)(phys_base | offset);
}

void *paging_get_virtual(page_directory_entry *page_dir, void *physAddr)
{
    uint32_t phys = (uint32_t)physAddr & ~0xFFF; // page-align

    for (uint32_t pdi = 0; pdi < 1024; pdi++)
    {
        if (!page_dir[pdi].flags.flags_bitmap.present)
        {
            continue;
        }

        page_table_entry *pt = (page_table_entry *)(page_dir[pdi].frame << 12);

        for (uint32_t pti = 0; pti < 1024; pti++)
        {
            if (!pt[pti].flags.flags_bitmap.present)
            {
                continue;
            }

            if ((pt[pti].frame << 12) == phys)
            {
                // Reconstruct the virtual address from the indices
                return (void *)((pdi << 22) | (pti << 12));
            }
        }
    }

    return NULL; // not mapped
}

static inline uint32_t phys_to_page_index(void *physAddr)
{
    return ((uint32_t)physAddr) / PAGE_SIZE;
}

static inline void *page_index_to_phys(uint32_t page_index)
{
    return (void *)(page_index * PAGE_SIZE);
}

void paging_init()
{
    frame_init();

    frame_alloc_region(0x0, 0x00100000); // null/BIOS low page
}

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
void *paging_alloc_frame()
{
    int i = frame_alloc_frame();
    if (i == 0)
    {
        write_error(LVL_CRITICAL, "Paging", "crit error: Out of pages");
        log_debug("Paging", "why and how the fuck did that happen");
        return NULL;
    }
    log_debug(MODULE, "got frame index %u", i / PAGE_SIZE);

    return (void *)i;
}

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
void *paging_alloc_frame_at(void *physAddr)
{
    uint32_t frame_index = phys_to_page_index(physAddr);

    if (frame_get_frame(frame_index))
    {
        write_error(LVL_ERROR, "Paging", "error: Frame %u already claimed", frame_index);
        log_debug(MODULE, "Allocating for %p", physAddr);
        return NULL; // already claimed
    }
    log_debug(MODULE, "got frame index %u", frame_index);

    frame_alloc_at((uint32_t)physAddr);
    return (void *)((uint32_t)physAddr & ~0xFFF); // return page-aligned addr
}

// Free a physical frame by its physical address.
void paging_free_frame(void *physAddr)
{
    uint32_t frame_index = phys_to_page_index(physAddr);

    if (!frame_get_frame(frame_index))
    {
        write_error(LVL_ERROR, "Paging", "Frame %u at %p not allocated", frame_index / PAGE_SIZE, physAddr);
        return;
    }

    frame_mark_frame_free(frame_index);
}

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(page_directory_entry *page_dir, void *virtAddr, void *physAddr, uint32_t flags)
{
    uint32_t virt = (uint32_t)virtAddr;
    uint32_t page_directory_index = GETPAGEDIRECTORYINDEX(virt);
    uint32_t page_table_index = GETPAGETABLEINDEX(virt);

    page_directory_entry *page_directory_entry = &page_dir[page_directory_index];

    uint32_t a = (uint32_t)page_directory_entry->frame;

    uint32_t _flags = flags;
    {
        if (flags == -1)
        {
            _flags = PAGE_WRITABLE;
        }
        _flags |= PAGE_PRESENT;
    }

    if (!page_directory_entry->flags.flags_bitmap.present)
    {
        void *page_table_phys = paging_alloc_frame();
        if (page_table_phys == NULL)
        {
            log_crit("Paging", "out of memory");
            KernelPanic("Paging", "out of memory");
        }
        void *page_table_virt = page_table_phys + KERNEL_VIRT_BASE;
        memset(page_table_virt, 0, PAGE_SIZE);

        page_directory_entry->frame = ((uint32_t)(page_table_phys) >> 12);
        page_directory_entry->flags.flags_byte |= _flags;
    }

    page_table_entry *page_table = (page_table_entry *)(void *)(page_directory_entry->frame << 12) + KERNEL_VIRT_BASE;
    page_table[page_table_index].frame = (uint32_t)physAddr >> 12;

page_table[page_table_index].flags.flags_byte = _flags;

    // Invalidate the TLB entry for this address
    __asm__ volatile("invlpg (%0)" ::"r"(virtAddr) : "memory");
}

// Unmap a single virtual page (does NOT free the underlying frame).
void paging_unmap_page(page_directory_entry *page_dir, void *virtAddr)
{
    uint32_t virt = (uint32_t)virtAddr;
    uint32_t page_directory_index = GETPAGEDIRECTORYINDEX(virt);
    uint32_t page_table_index = GETPAGETABLEINDEX(virt);

    page_directory_entry *page_directory_entry = &page_dir[page_directory_index];
    if (!page_directory_entry->flags.flags_bitmap.present)
    {
        write_error(LVL_ERROR, "Paging", "Error: Page directory %u has not been allocated", page_directory_index);
    }

    page_table_entry *page_table = (page_table_entry *)paging_get_virtual(page_dir, (void *)(page_directory_entry->frame << 12));
    FLAG_UNSET(page_table->flags.flags_byte, PAGE_PRESENT);
    FLAG_UNSET(page_directory_entry->flags.flags_byte, PAGE_PRESENT);

    // Invalidate the TLB entry for this address
    __asm__ volatile("invlpg (%0)" ::"r"(virtAddr) : "memory");
}

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
void *paging_alloc_and_map(page_directory_entry *page_dir, void *virtAddr, uint32_t flags)
{
    void *phys = paging_alloc_frame();
    if (phys == NULL)
        return NULL; // OOM

    paging_map_page(page_dir, virtAddr, phys, flags);
    return phys;
}

void *paging_alloc_and_map_region(page_directory_entry *page_dir, void *virtAddr, size_t size, uint32_t flags)
{
    uint32_t virt = (uint32_t)virtAddr;
    void *phys = paging_alloc_frame();

    // Round up to page boundary
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages; i++)
    {
        paging_map_page(page_dir, (void *)virt, phys, flags);
        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
    }
    return phys;
}

// Map a contiguous physical region [physAddr, physAddr + size) to
// [virtAddr, virtAddr + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
void paging_map_region(page_directory_entry *page_dir, void *virtAddr, void *physAddr, size_t size, uint32_t flags)
{
    uint32_t virt = (uint32_t)virtAddr;
    uint32_t phys = (uint32_t)physAddr;

    // Round up to page boundary
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages; i++)
    {
        paging_map_page(page_dir, (void *)virt, (void *)phys, flags);
        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
    }
}

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(page_directory_entry *page_dir, void *virtAddr, size_t size)
{
    uint32_t virt = (uint32_t)virtAddr;
    // error things maybe?
    /*     uint32_t phys = (uint32_t)paging_get_physical(virtAddr);
    if (phys == NULL)
    {
        write_error(LVL_ERROR, "Paging", "Page at virt %p not allocated", virt);
        return;
    } */

    // Round up to page boundary
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages; i++)
    {
        paging_unmap_page(page_dir, (void *)virt);
        virt += PAGE_SIZE;
    }
}

page_directory_entry *paging_create_user_directory()
{
    uint32_t phys = frame_alloc_frame();

    page_directory_entry *pd = (page_directory_entry *)phys;
    memset(pd, 0, PAGE_SIZE);

    for (int i = 768; i < 1024; i++)
        pd[i] = kernel_page_directory[i];

    return pd;
}
