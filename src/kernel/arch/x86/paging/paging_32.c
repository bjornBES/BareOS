/*
 * File: paging.c
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#ifdef __i686__
#include "paging.h"
#include "paging_32.h"
#include "debug/debug.h"
#include "frame.h"

#include "libs/stdio.h"
#include "libs/malloc.h"
#include "libs/memory.h"
#include "libs/string.h"

#include <util/binary.h>

#define PAGE_SIZE 4096

#define MODULE "PAGING"

bool paging_print_out = true;

page_directory32 *kernel_page = 0;
uint32_t page_dir_loc = 0;

uint32_t *last_page = 0;

virt_addr phys_to_virt(phys_addr phys)
{
    // 0x00101000
    // 0x00100000
    // 0x00012c00
    if (phys >= 0x100000)
        return (virt_addr)(phys + KERNEL_VIRT_BASE - KERNEL_PHYS_BASE); // kernel higher-half
    else
        return (virt_addr)phys; // identity mapped low memory

    return NULL;
}

void *paging_get_physical(paging_page *page_dir, virt_addr virtualaddr)
{
    uint32_t pdi = GETPAGEDIRECTORYINDEX(virtualaddr);
    uint32_t pti = GETPAGETABLEINDEX(virtualaddr);
    uint32_t offset = (uint32_t)virtualaddr & 0xFFF;

    page_directory_entry32 *pde = &page_dir->entries[pdi];
    if (!(pde->flags.flags_bitmap.present))
    {
        log_warn("Paging", "page directory entry [%u] was not pressent with virt %p", pdi, virtualaddr);
        log_warn("Paging", "pd[%u].pt[%d]", pdi, pti);
        return NULL;
    }

    uint32_t pt_phys = pde->frame << 12;
    page_table32 *pt = phys_to_virt((void *)(pt_phys));
    page_table_entry *pte = &pt->entries[pti];

    if (!(pte->flags.flags_bitmap.present))
    {
        log_warn("Paging", "page table was not pressent with virt %p", virtualaddr);
        return NULL;
    }

    uint32_t phys_base = pte->frame << 12;
    return (void *)(phys_base | offset);
}

void *paging_get_virtual(paging_page *page_dir, phys_addr physAddr)
{
    uint32_t phys = (uint32_t)physAddr & ~0xFFF; // page-align

    for (uint32_t pdi = 0; pdi < 1024; pdi++)
    {
        if (!page_dir->entries[pdi].flags.flags_bitmap.present)
        {
            continue;
        }

        page_table32 *pt = (page_table32 *)phys_to_virt((void *)(page_dir->entries[pdi].frame << 12));

        for (uint32_t pti = 1023; pti != 0; pti--)
        {
            page_table_entry pte = pt->entries[pti];
            if (!pte.flags.flags_bitmap.present)
            {
                continue;
            }

            // log_debug(MODULE, "%x == %x", (pte.frame << 12), phys);

            if ((pte.frame << 12) == phys)
            {
                log_debug(MODULE, "Done got v0x%x | v0x%x = v0x%x from p0x%x", (pdi << 22), (pti << 12), (pdi << 22) | (pti << 12), physAddr);
                log_debug(MODULE, "Done got pdi %u | pti %u from p0x%x", pdi, pti, physAddr);
                // Reconstruct the virtual address from the indices
                return (void *)((pdi << 22) | (pti << 12));
            }
        }
    }
    log_warn("Paging", "could not get virtual from %p", phys);
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

void paging_init32()
{
    // map new Page_dir[0]
    log_debug(MODULE, "Mapping new Page_dir[0]");
    // Allocate a new page table from kernel-mapped frames
    void *pt_low_phys = paging_alloc_frame();
    page_table32 *pt_low = (page_table32 *)phys_to_virt(pt_low_phys);
    memset(pt_low, 0, PAGE_SIZE);
    log_debug(MODULE, "low page table at v%p;p%p", pt_low, pt_low_phys);

    log_debug(MODULE, "Setting new pt");
    // Identity map 0x0 - 0x3FFFFF (same as bootloader did)
    for (int i = 0; i < 1024; i++)
    {
        pt_low->entries[i].raw = (i * 0x1000) | 0x03;
    }
    log_debug(MODULE, "Done with copy");

    log_debug(MODULE, "installing low");
    // Install into kernel page directory
    kernel_page->entries[0].raw = ((uint32_t)pt_low_phys) | 0x03;
    log_debug(MODULE, "Done with the low");

    log_debug(MODULE, "Flush TLB");
    // Flush TLB
    __asm__ volatile("mov %%cr3, %%eax\n"
                     "mov %%eax, %%cr3" ::: "eax");

    log_debug(MODULE, "PDE[0] rebuilt at phys %p", pt_low_phys);
#define PAGING_TEST_PHYS_TO_VIRT(p) log_debug(MODULE, "\t-v%p > p%p", paging_get_virtual(kernel_page, (void *)p), p)
#define PAGING_TEST_VIRT_TO_PHYS(p) log_debug(MODULE, "\t-p%p > v%p", paging_get_physical(kernel_page, (void *)p), p)
    log_debug(MODULE, "test PDE[0]");
    PAGING_TEST_VIRT_TO_PHYS(0x1000);

    // map new Page_dir[768]
    // 1. Allocate new page table in kernel-mapped region (>= 0x100000)
    void *pt_kernel_phys = paging_alloc_frame();
    page_table32 *pt_kernel = (page_table32 *)phys_to_virt(pt_kernel_phys);
    memset(pt_kernel, 0, PAGE_SIZE);

    // 2. Copy ALL existing entries from old pageTableKernel into new one
    // Old one is at 0x16000, identity mapped
    for (int i = 0; i < 1024; i++)
    {
        pt_kernel->entries[i].raw = ((0x00100000 + i * 0x1000)) | 0x03;
    }

    // 3. NOW atomically swap PDE[768] to point to new table
    kernel_page->entries[768].raw = ((uint32_t)pt_kernel_phys) | 0x03;

    // 4. Flush TLB - this is the moment of truth
    __asm__ volatile("mov %%cr3, %%eax\n"
                     "mov %%eax, %%cr3" ::: "eax");
    log_debug(MODULE, "PDE[768] rebuilt at phys %p", pt_kernel_phys);

    log_debug(MODULE, "test PDE[768]");
    PAGING_TEST_VIRT_TO_PHYS(0x00000000c000ce00);
    PAGING_TEST_VIRT_TO_PHYS(0x00000000c0010000);
    PAGING_TEST_VIRT_TO_PHYS(0x00000000c0012f00);
    PAGING_TEST_VIRT_TO_PHYS(0x00000000c004c000);
    PAGING_TEST_PHYS_TO_VIRT(0x0014c000);

#undef PAGING_TEST_PHYS_TO_VIRT
#undef PAGING_TEST_VIRT_TO_PHYS
}

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
phys_addr paging_alloc_frame()
{
    phys_addr i = frame_alloc_frame();
    if (i == 0)
    {
        write_error(LVL_CRITICAL, "Paging", "crit error: Out of pages got %u", i);
        log_debug("Paging", "why and how the fuck did that happen");
        return NULL;
    }
    if (i < KERNEL_PHYS_BASE)
    {
        log_warn("MAIN", "=========== nr.2 ===========");
        // frame_dump_bitmap();
    }
    log_debug(MODULE, "got frame index %u", i / PAGE_SIZE);

    return i;
}

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
void *paging_alloc_frame_at(phys_addr physAddr)
{
    uint32_t frame_index = physAddr / PAGE_SIZE;

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
void paging_free_frame(phys_addr physAddr)
{
    uint32_t frame_index = (uint32_t)physAddr / PAGE_SIZE;

    if (!frame_get_frame(frame_index))
    {
        write_error(LVL_ERROR, "Paging", "Frame %u at %p not allocated", frame_index / PAGE_SIZE, physAddr);
        return;
    }

    frame_mark_frame_free(frame_index);
}

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(paging_page *page_paging, virt_addr virtAddr, phys_addr physAddr, uint32_t flags)
{
    virt_addr virt = (virt_addr)virtAddr;
    uint32_t page_directory_index = GETPAGEDIRECTORYINDEX(virt);
    uint32_t page_table_index = GETPAGETABLEINDEX(virt);
    if (paging_print_out)
    {
        log_debug(MODULE, "map_page page_dir=%p virt=%p phys=%p flags=%x pdi=%u pti=%u", page_paging, virtAddr, physAddr, flags, page_directory_index, page_table_index);
    }

    page_directory32 *page_dir = (page_directory32*)page_paging->page_dir;
    page_directory_entry32 *page_directory_entry = phys_to_virt(&page_dir->entries[page_directory_index]);
    if (paging_print_out)
    {
        log_debug(MODULE, "page_directory_entry at v%p | p%p", page_directory_entry, &page_dir[page_directory_index]);
    }

    uint32_t _flags = flags;
    {
        if (flags == -1)
        {
            _flags = PAGE_WRITABLE;
        }
        _flags |= PAGE_PRESENT;
    }
    if (paging_print_out)
    {
        log_debug(MODULE, "resolved flags=%x", _flags);
    }

    if (!page_directory_entry->present)
    {
        if (paging_print_out)
        {
            log_info(MODULE, "pdi=%u not present, allocating new page table", page_directory_index);
        }

        void *page_table_phys = paging_alloc_frame();
        if (page_table_phys == NULL)
        {
            log_crit(MODULE, "out of memory");
            KernelPanic(MODULE, "out of memory");
        }
        if (paging_print_out)
        {
            log_debug(MODULE, "new page table phys=%p", page_table_phys);
        }

        void *page_table_virt = phys_to_virt(page_table_phys);
        if (paging_print_out)
        {
            log_debug(MODULE, "new page table virt=%p", page_table_virt);
        }

        if (page_table_virt == NULL)
        {
            log_crit(MODULE, "paging_get_virtual returned NULL for new page table phys=%p", page_table_phys);
            KernelPanic(MODULE, "could not get virtual address for new page table");
        }

        memset(page_table_virt, 0, PAGE_SIZE);
#ifdef __x86_64__ // shut up gcc
        page_directory_entry->frame = (uint32_t)((uint64_t)(page_table_phys) >> 12);
#else
        page_directory_entry->frame = ((uint32_t)(page_table_phys) >> 12);
#endif
        page_directory_entry->raw |= _flags;

        if (paging_print_out)
        {
            log_info(MODULE, "installed PDE[%u] -> phys=%p flags=%x", page_directory_index, page_table_phys, _flags);
        }
    }
    else
    {
        if (paging_print_out)
        {
            log_debug(MODULE, "pdi=%u already present frame=%x", page_directory_index, page_directory_entry->frame);
        }
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    phys_addr frame_addr;
#ifdef __x86_64__ // shut up gcc
    frame_addr = (phys_addr)(uint64_t)(page_directory_entry->frame << 12);
#else
    frame_addr = (phys_addr)(page_directory_entry->frame << 12);
#endif

    page_table32 *pt = (page_table32 *)phys_to_virt((phys_addr)frame_addr);
#pragma GCC diagnostic pop
    if (paging_print_out)
    {
        log_debug(MODULE, "page table virt=%p", pt);
    }

    if (pt == NULL)
    {
        log_crit(MODULE, "paging_get_virtual returned NULL for page table frame=%x", page_directory_entry->frame);
        KernelPanic(MODULE, "could not get virtual address for page table");
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    pt->entries[page_table_index].frame = (uint32_t)physAddr >> 12;
    pt->entries[page_table_index].flags.flags_byte = _flags;
#pragma GCC diagnostic pop

    if (paging_print_out)
    {
        log_info(MODULE, "mapped virt=%p -> phys=%p (pdi=%u pti=%u flags=%x)", virtAddr, physAddr, page_directory_index, page_table_index, _flags);
    }

    // Invalidate the TLB entry for this address
    __asm__ volatile("invlpg %0" ::"r"(virtAddr) : "memory");
}

// Unmap a single virtual page (does NOT free the underlying frame).
void paging_unmap_page(paging_page *paging_page, virt_addr virtAddr)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    virt_addr virt = virtAddr;
    uint32_t page_directory_index = GETPAGEDIRECTORYINDEX(virt);
    uint32_t page_table_index = GETPAGETABLEINDEX(virt);

    page_directory32 *page_dir = (page_directory32 *)paging_page->page_dir;

    page_directory_entry32 page_directory_entry = page_dir->entries[page_directory_index];
    if (!page_directory_entry.present)
    {
        write_error(LVL_ERROR, "Paging", "Error: Page directory %u has not been allocated", page_directory_index);
    }

    phys_addr frame_addr = (phys_addr)(page_directory_entry.frame << 12);

    page_table32 *pt = (page_table32 *)paging_get_virtual(paging_page, frame_addr);
    page_table_entry pte = pt->entries[page_table_index];
    FLAG_UNSET(pte.raw, PAGE_PRESENT);
    FLAG_UNSET(page_directory_entry.raw, PAGE_PRESENT);
#pragma GCC diagnostic pop
    // Invalidate the TLB entry for this address
    __asm__ volatile("invlpg %0" ::"r"(virtAddr) : "memory");
}

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
phys_addr paging_alloc_and_map(paging_page *page_dir, virt_addr virtAddr, uint32_t flags)
{
    phys_addr phys = paging_alloc_frame();
    if (phys == NULL)
        return NULL; // OOM

    paging_map_page(page_dir, virtAddr, phys, flags);
    return phys;
}

void *paging_alloc_and_map_region(paging_page *page_dir, virt_addr virtAddr, size_t size, uint32_t flags)
{
    virt_addr virt = virtAddr;
    void *org_phys = paging_alloc_frame();
    void *phys = org_phys;

    // Round up to page boundary
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages; i++)
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
void paging_map_region(paging_page *page_dir, virt_addr virtAddr, phys_addr physAddr, size_t size, uint32_t flags)
{
    virt_addr virt = virtAddr;
    phys_addr phys = physAddr;

    // Round up to page boundary
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages; i++)
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
        paging_unmap_page(page_dir, (virt_addr)virt);
        virt += PAGE_SIZE;
    }
}

void paging_create_user_directory(void *_proc)
{
    process *proc = (process *)_proc;
    phys_addr phys = frame_alloc_frame();

    page_directory32 *pd = (page_directory32 *)phys_to_virt(phys);
    memset(pd, 0, PAGE_SIZE);

    for (int i = 768; i < 1024; i++)
    {
        // kernel higher half
        pd->entries[i] = kernel_page->entries[i];
    }

    proc->page_dir_virt.page_dir = pd;
    proc->page_dir_phys.page_dir = phys;
}

#endif
