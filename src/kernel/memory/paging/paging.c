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
#include "kernel.h"

#include "memory/pmm/pmm.h"
#include "debug/debug.h"

#include "libs/malloc.h"
#include "libs/stdio.h"

#include <util/binary.h>

#define MODULE "PAGING"

int kernel_page_index;
bool paging_enabled;
extern char __kernel_pages;

virt_addr paging_phys_to_virt(phys_addr phys)
{
    return (virt_addr)((uint64_t)phys + (uint64_t)(KERNEL_VIRT_BASE - KERNEL_PHYS_BASE)); // kernel higher-half
}
phys_addr paging_virt_to_phys(virt_addr virt)
{
    return (virt_addr)((uint64_t)virt - (uint64_t)(KERNEL_VIRT_BASE - KERNEL_PHYS_BASE)); // kernel higher-half
}

void paging_init(boot_params *bootParams)
{
    paging_enabled = false;
    pmm_init(bootParams);

    int count = bootParams->e820Count;
    E820_entry e820_entries[MAX_E820_ENTRIES];
    int e820_entries_index = 0;
    for (size_t i = 0; i < count; i++)
    {
        E820_entry *entry = &bootParams->e820Entries[i];
        log_debug(MODULE, "E820: base=0x%llx length=0x%llx type=0x%x", entry->addr, entry->size, entry->type);
        if (entry->type != MEMORY_AVAILABLE)
        {
            memcpy(&e820_entries[e820_entries_index], entry, sizeof(E820_entry));
            e820_entries_index++;
        }
    }

    paging_x86_init(bootParams, start, end);
    log_debug(MODULE, "paging done");

    pmm_map();

    paging_alloc_frame_region((phys_addr)0, (size_t)KERNEL_PHYS_BASE);
    paging_alloc_frame_region((phys_addr)KERNEL_PHYS_BASE, (size_t)KERNEL_SIZE);

    for (size_t i = 0; i < e820_entries_index; i++)
    {
        E820_entry *entry = &e820_entries[i];
        log_debug(MODULE, "E820: base=0x%llx length=0x%llx type=0x%x", entry->addr, entry->size, entry->type);
        if (entry->type != MEMORY_AVAILABLE)
        {
            paging_alloc_frame_region((phys_addr)entry->addr, (size_t)entry->size);
        }
    }

    paging_enabled = true;
    paging_print_out = true;
    paging_map_region(kernel_page, (virt_addr)0, (phys_addr)0, sizeof(boot_params)+PAGE_SIZE, -1);
    paging_print_out = false;

    uint32_t some = bootParams->memory.memLower;

    paging_x86_init(bootParams, start, end);
    log_debug(MODULE, "paging done");

    heap_init();
}

phys_addr paging_get_kernel_frame()
{
    if (!paging_enabled)
    {
        void *frame = (void *)&__kernel_pages;
        frame += (kernel_page_index * 4096);
        kernel_page_index++;
        return (phys_addr)frame;
    }
    return paging_alloc_frame();
}

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
phys_addr paging_alloc_frame()
{
    phys_addr addr = pmm_alloc_frame();
    if (addr < KERNEL_PHYS_BASE)
    {
        log_warn(MODULE, "Something may be wrong");
    }

    {
        virt_addr addr_virt = phys_to_virt_auto(addr);
        uint64_t *test = (uint64_t *)addr_virt;
        uint64_t save_test = *test;
        *test = 0xFF00FF00;
        while (*test != 0xFF00FF00)
        {
            addr = paging_alloc_frame();
            if (!addr)
            {
                return NULL;
            }
            addr_virt = phys_to_virt_auto(addr);
            test = (uint64_t *)addr_virt;
            *test = 0xFF00FF00;
        }
        *test = save_test;
    }

    log_debug(MODULE, "got frame addr %p", addr);

    return addr;
}

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
phys_addr paging_alloc_frame_at(phys_addr physAddr)
{
    phys_addr addr = pmm_alloc_at(physAddr);
    if (addr == NULL)
    {
        kernel_panic("out of phys frames from paging_alloc_frame_at");
        return NULL;
    }
    log_debug(MODULE, "got frame addr %p", addr);

    return (phys_addr)((uint32_64)addr & ~0xFFF); // return page-aligned addr
}

// Allocate the specific physical region of frames containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
phys_addr paging_alloc_frame_region(phys_addr physAddr, size_t size)
{
    phys_addr addr = pmm_alloc_region(physAddr, physAddr + size);
    return (phys_addr)((uint32_64)addr & ~0xFFF); // return page-aligned addr
}

// Free a physical frame by its physical address.
void paging_free_frame(phys_addr physAddr)
{
    pmm_free_frame(physAddr);
}

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
phys_addr paging_alloc_and_map(paging_page page_dir, virt_addr virtAddr, paging_flags flags)
{
    phys_addr phys = paging_alloc_frame();
    if (phys == NULL)
        return NULL; // OOM

    paging_map_page(page_dir, virtAddr, phys, flags);
    return phys;
}

phys_addr paging_alloc_and_map_region(paging_page page_dir, virt_addr virtAddr, size_t size, paging_flags flags)
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
void paging_map_region(paging_page page_dir, virt_addr virtAddr, phys_addr physAddr, size_t size, paging_flags flags)
{
    virt_addr virt = virtAddr;
    phys_addr phys = physAddr;

    // Round up to page boundary
    uint32_64 pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    log_info(MODULE, "mapping region [v%p-v%p] to [p%p-p%p] size in pages is %u",
             virtAddr, virtAddr + pages * PAGE_SIZE,
             physAddr, physAddr + pages * PAGE_SIZE, pages);
    for (uint32_64 i = 0; i < pages; i++)
    {
        paging_map_page(page_dir, virt, phys, flags);
        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
    }

    reload_pages();
}

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(paging_page page_dir, virt_addr virtAddr, size_t size)
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

void paging_info()
{
}