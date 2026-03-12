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
#include <memory.h>
#include <string.h>
#include <util/binary.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 0x10000

#define KERNEL_VIRT_BASE 0xC0000000
#define KERNEL_PHYS_BASE 0x00100000 // 1MB

uint8_t page_bitmap[MAX_PAGES / 8]; // 1 bit per page

page_directory_entry *page_directory = 0;
uint32_t page_dir_loc = 0;

uint32_t *last_page = 0;

void *paging_get_physical(void *virtualaddr)
{
    uint32_t pdi = GETPAGEDIRECTORYINDEX(virtualaddr);
    uint32_t pti = GETPAGETABLEINDEX(virtualaddr);
    uint32_t offset = (uint32_t)virtualaddr & 0xFFF;

    void* page_dir = (void*)page_directory;
    uint32_t *pd = (uint32_t*)page_dir;
    if (!(pd[pdi] & 0x1))
    {
        return NULL;
    }

    uint32_t *pt = (uint32_t*)(pd[pdi] & ~0xFFF);

    if (!(pt[pdi] & 0x1))
    {
        return NULL;
    }

    uint32_t phys_base = pt[pti] & ~0xFFF;
    return (void *)(phys_base | offset);
}

void *paging_get_virtual(void *physAddr) {
    uint32_t phys = (uint32_t)physAddr & ~0xFFF; // page-align

    for (uint32_t pdi = 0; pdi < 1024; pdi++)
    {
        if (!page_directory[pdi].flags.flags_bitmap.present)
        {
            continue;
        }

        page_table_entry *pt = (page_table_entry *)(page_directory[pdi].frame << 12);

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
    return (uint32_t)physAddr / PAGE_SIZE;
}

static inline void *page_index_to_phys(uint32_t page_index)
{
    return (void *)(page_index * PAGE_SIZE);
}

void paging_init(void *ptable)
{
    int page_table_index = GETPAGEDIRECTORYINDEX(ptable);
    paging_mark_page_used(page_table_index);
    last_page = ptable;

    paging_mark_page_used(0); // 0x000000 - 0x3FFFFF
    paging_mark_page_used(1); // 0x400000 - 0x7FFFFF
    paging_mark_page_used(2); // 0x800000 - 0xBFFFFF
    paging_mark_page_used(3); // 0xC00000 - 0xFFFFFF
    paging_mark_page_used(0x300); // kernel 0xC0000000 -> 0x00100000
    paging_map_region((void *)0x400000, (void *)0x400000, 1024, -1); // -1 for PAGE_PRESENT | PAGE_WRITABLE
    paging_map_region((void *)0x800000, (void *)0x800000, 1024, -1); // -1 for PAGE_PRESENT | PAGE_WRITABLE
}

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
void *paging_alloc_frame()
{
    int i = paging_find_free_page();
    if (i == -1)
    {
        write_error(LVL_CRITICAL, "Paging", "crit error: Out of pages");
        log_debug("Paging", "why and how the fuck did that happen");
        return NULL;
    }

    paging_mark_page_used(i);

    return (void *)(i * PAGE_SIZE);
}

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
void *paging_alloc_frame_at(void *physAddr)
{
    uint32_t page_index = phys_to_page_index(physAddr);

    if (paging_check_page(page_index))
        return NULL; // already claimed

    paging_mark_page_used(page_index);
    return (void *)((uint32_t)physAddr & ~0xFFF); // return page-aligned addr
}

// Free a physical frame by its physical address.
void paging_free_frame(void *physAddr)
{
    uint32_t page_index = phys_to_page_index(physAddr);

    if (!paging_check_page(page_index))
    {
        write_error(LVL_ERROR, "Paging", "Page %u at %p not allocated", page_index, physAddr);
    }

    paging_mark_page_free(page_index);
}

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(void *virtAddr, void *physAddr, uint32_t flags)
{
    uint32_t virt = (uint32_t)virtAddr;
    uint32_t page_directory_index = GETPAGEDIRECTORYINDEX(virt);
    uint32_t page_table_index = GETPAGETABLEINDEX(virt);

    page_directory_entry *page_directory_entry = &page_directory[page_directory_index];

    if (!page_directory_entry->flags.flags_bitmap.present)
    {
        void *page_table_phys = paging_alloc_frame();
        if (page_table_phys == NULL)
        {
            log_crit("Paging", "out of memory");
            KernelPanic("Paging", "out of memory");
        }
        void *page_table_virt = paging_get_virtual(page_table_phys);
        memset(page_table_virt, 0, PAGE_SIZE);

        page_directory_entry->frame = ((uint32_t)(page_table_phys) >> 12);
        page_directory_entry->flags.flags_byte |= (PAGE_PRESENT | PAGE_WRITABLE);
    }

    page_table_entry *page_table = (page_table_entry *)paging_get_virtual((void *)(page_directory_entry->frame << 12));
    page_table[page_table_index].frame = (uint32_t)physAddr >> 12;

    {
        uint32_t _flags = flags;
        if (flags == -1)
        {
            _flags = PAGE_WRITABLE;
        }
        _flags |= PAGE_PRESENT;
        page_table[page_table_index].flags.flags_byte = _flags;
    }

    // Invalidate the TLB entry for this address
    __asm__ volatile("invlpg (%0)" ::"r"(virtAddr) : "memory");
}

// Unmap a single virtual page (does NOT free the underlying frame).
void paging_unmap_page(void *virtAddr)
{
    uint32_t virt = (uint32_t)virtAddr;
    uint32_t page_directory_index = GETPAGEDIRECTORYINDEX(virt);
    uint32_t page_table_index = GETPAGETABLEINDEX(virt);

    page_directory_entry *page_directory_entry = &page_directory[page_directory_index];
    if (!page_directory_entry->flags.flags_bitmap.present)
    {
        write_error(LVL_ERROR, "Paging", "Error: Page directory %u has not been allocated", page_directory_index);
    }

    page_table_entry *page_table = (page_table_entry *)paging_get_virtual((void *)(page_directory_entry->frame << 12));
    FLAG_UNSET(page_table->flags.flags_byte, PAGE_PRESENT);
    FLAG_UNSET(page_directory_entry->flags.flags_byte, PAGE_PRESENT);

    // Invalidate the TLB entry for this address
    __asm__ volatile("invlpg (%0)" ::"r"(virtAddr) : "memory");
}

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
void *paging_alloc_and_map(void *virtAddr, uint32_t flags)
{
    void *phys = paging_alloc_frame();
    if (phys == NULL)
        return NULL; // OOM

    paging_map_page(virtAddr, phys, flags);
    return phys;
}

// Map a contiguous physical region [physAddr, physAddr + size) to
// [virtAddr, virtAddr + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
void paging_map_region(void *virtAddr, void *physAddr, size_t size, uint32_t flags)
{
    uint32_t virt = (uint32_t)virtAddr;
    uint32_t phys = (uint32_t)physAddr;

    // Round up to page boundary
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages; i++)
    {
        paging_map_page((void *)virt, (void *)phys, flags);
        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
    }
}

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(void *virtAddr, size_t size)
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
        paging_unmap_page((void *)virt);
        virt += PAGE_SIZE;
    }
}

int paging_find_free_page()
{
    for (size_t i = 0; i < MAX_PAGES; i++)
    {
        if (!paging_check_page(i))
        {
            return i;
        }
    }
    return -1;
}

inline int paging_check_page(uint32_t page_index)
{
    return BIT_GET(page_bitmap[page_index / 8], page_index % 8);
}

inline void paging_mark_page_used(uint32_t page_index)
{
    BIT_SET(page_bitmap[page_index / 8], (page_index % 8));
}

inline void paging_mark_page_free(uint32_t page_index)
{
    BIT_UNSET(page_bitmap[page_index / 8], (page_index % 8));
}
