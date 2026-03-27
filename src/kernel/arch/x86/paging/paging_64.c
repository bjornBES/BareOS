/*
 * File: paging_64.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"
#include "paging_64.h"

#define MODULE "PAGING_64"

#define PAGE_SIZE 4096

#define GET_PML4_IDX(va) (((va) >> 39) & 0x1FF)
#define GET_PDPT_IDX(va) (((va) >> 30) & 0x1FF)
#define GET_PD_IDX(va) (((va) >> 21) & 0x1FF)
#define GET_PT_IDX(va) (((va) >> 12) & 0x1FF)
#define GET_PAGE_OFF(va) ((va) & 0xFFF)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL
#define PAGE_FLAGS_MASK 0xFFF0000000000FFFULL

typedef enum
{
    PAGING_LEVEL_PML4 = 4,
    PAGING_LEVEL_PDPT = 3,
    PAGING_LEVEL_PD = 2,
    PAGING_LEVEL_PT = 1,
} paging_level;

__attribute__((aligned(4096))) cr3_t kernel_cr3;

paging_page *kernel_page;

bool paging_print_out = true;

virt_addr phys_to_virt(phys_addr phys)
{
    if ((uint64_t)phys >= 0x100000)
    {
        return (virt_addr)((uint64_t)phys + (uint64_t)(KERNEL_VIRT_BASE - KERNEL_PHYS_BASE)); // kernel higher-half
    }

    return (virt_addr)phys; // identity mapped low memory
}

void paging_init64()
{
    __asm__ volatile("mov %0, cr3" : "=r"(kernel_cr3));

    page_map_level_4 *kernel_pml4 = (page_map_level_4 *)paging_alloc_frame();
    page_dpt *kernel_pdpt = (page_dpt *)paging_alloc_frame();
    page_directory64 *kernel_pd = (page_directory64 *)paging_alloc_frame();

    kernel_pd->e[0].raw = 1;
    kernel_pd->e[0].present = 1;
    kernel_pd->e[0].writable = 1;
    kernel_pd->e[0].ps_or_pat = 1;
    kernel_pd->e[0].addr = (uint64_t)KERNEL_PHYS_BASE >> 12;

    kernel_pdpt->e[510].present = 1;
    kernel_pdpt->e[510].writable = 1;
    kernel_pdpt->e[510].addr = (uint64_t)phys_to_virt((phys_addr)((uint64_t)kernel_pd >> 12));

    kernel_pml4->e[511].present = 1;
    kernel_pml4->e[511].writable = 1;
    kernel_pml4->e[511].addr = (uint64_t)phys_to_virt((phys_addr)((uint64_t)kernel_pdpt >> 12));

    kernel_cr3.pml4_addr = (uint64_t)phys_to_virt((phys_addr)((uint64_t)kernel_pml4 >> 12));
    kernel_page->page_dir = kernel_pml4;
}

/* ===== HELPER FUNCTIONS ===== */
static int paging64_table_is_empty(page_table_entry64 *entries)
{
    for (int i = 0; i < 512; i++)
        if (entries[i].present)
            return 0;
    return 1;
}

// Returns a pointer to the entry at the requested level, or NULL on failure.
// If alloc=1, missing intermediate tables are allocated and zeroed.
// If alloc=0, returns NULL if any intermediate table is missing.
page_table_entry64 *paging64_get_entry(paging_page *paging_page, virt_addr virtAddr, paging_level target_level, bool alloc)
{
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virtAddr),
        GET_PDPT_IDX((uint64_t)virtAddr),
        GET_PD_IDX((uint64_t)virtAddr),
        GET_PT_IDX((uint64_t)virtAddr),
    };

    page_table_entry64 *table = ((page_map_level_4 *)paging_page->page_dir)->e;

    for (int level = 4; level > (int)target_level; level--)
    {
        page_table_entry64 *entry = &table[idx[4 - level]];

        if (!entry->present)
        {
            if (!alloc)
                return NULL;

            phys_addr new_phys = paging_alloc_frame();
            if (!new_phys)
                return NULL;

            memset((void *)phys_to_virt(new_phys), 0, PAGE_SIZE);

            entry->raw = 0;
            entry->addr = (uint64_t)new_phys >> 12;
            entry->present = 1;
            entry->writable = 1;
        }

        phys_addr next_phys = (phys_addr)((uint64_t)entry->addr << 12);
        table = ((page_map_level_4 *)phys_to_virt(next_phys))->e;
    }

    return &table[idx[4 - target_level]];
}

// Returns a pointer to the table AT the requested level, or NULL on failure.
// e.g. PAGING_LEVEL_PT returns the page_table64* that contains the PTE for virtAddr.
// If alloc=1, missing intermediate tables are allocated and zeroed.
void *paging64_get_table(paging_page *paging_page, virt_addr virtAddr, paging_level target_level, int alloc)
{
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virtAddr),
        GET_PDPT_IDX((uint64_t)virtAddr),
        GET_PD_IDX((uint64_t)virtAddr),
        GET_PT_IDX((uint64_t)virtAddr),
    };

    page_table_entry64 *table = ((page_map_level_4 *)paging_page->page_dir)->e;

    for (int level = 4; level >= (int)target_level; level--)
    {
        if (level == (int)target_level)
            return (void *)table; // cast to page_map_level_4/page_directory64/etc. at call site

        page_table_entry64 *entry = &table[idx[4 - level]];

        if (!entry->present)
        {
            if (!alloc)
                return NULL;

            phys_addr new_phys = paging_alloc_frame();
            if (!new_phys)
                return NULL;

            memset((void *)phys_to_virt(new_phys), 0, PAGE_SIZE);

            entry->raw = 0;
            entry->addr = (uint64_t)new_phys >> 12;
            entry->present = 1;
            entry->writable = 1;
        }

        phys_addr next_phys = (phys_addr)((uint64_t)entry->addr << 12);
        table = ((page_map_level_4 *)phys_to_virt(next_phys))->e;
    }

    return NULL; // unreachable
}

static inline page_map_level_4 *paging64_get_pml4(paging_page *p, virt_addr v, int alloc)
{
    return (page_map_level_4 *)paging64_get_table(p, v, PAGING_LEVEL_PML4, alloc);
}
static inline page_dpt *paging64_get_pdpt(paging_page *p, virt_addr v, int alloc)
{
    return (page_dpt *)paging64_get_table(p, v, PAGING_LEVEL_PDPT, alloc);
}
static inline page_directory64 *paging64_get_pd(paging_page *p, virt_addr v, int alloc)
{
    return (page_directory64 *)paging64_get_table(p, v, PAGING_LEVEL_PD, alloc);
}
static inline page_table64 *paging64_get_pt(paging_page *p, virt_addr v, int alloc)
{
    return (page_table64 *)paging64_get_table(p, v, PAGING_LEVEL_PT, alloc);
}

static inline page_table_entry64 *paging64_get_pml4_entry(paging_page *p, virt_addr v, int alloc)
{
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PML4, alloc);
}
static inline page_table_entry64 *paging64_get_pdpt_entry(paging_page *p, virt_addr v, int alloc)
{
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PDPT, alloc);
}
static inline page_table_entry64 *paging64_get_pd_entry(paging_page *p, virt_addr v, int alloc)
{
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PD, alloc);
}
static inline page_table_entry64 *paging64_get_pt_entry(paging_page *p, virt_addr v, int alloc)
{
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PT, alloc);
}

void paging_create_user_directory(void *_proc)
{
    FUNC_NOT_IMPLEMENTED(MODULE, "paging_create_user_directory");
}

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
phys_addr paging_get_physical(paging_page *page_dir, virt_addr virtAddr)
{
    FUNC_NOT_IMPLEMENTED(MODULE, "paging_get_physical");
    return NULL;
}

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
virt_addr paging_get_virtual(paging_page *page_dir, phys_addr physAddr)
{
    FUNC_NOT_IMPLEMENTED(MODULE, "paging_get_virtual");
    return NULL;
}

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(paging_page *page_paging, virt_addr virtAddr, phys_addr physAddr, paging_flags flags)
{
    virt_addr virt = (virt_addr)virtAddr;

    if (paging_print_out)
    {
        log_debug(MODULE, "map_page virt=%p phys=%p flags=%llx pdi=%u pti=%u", virtAddr, physAddr, flags,
                  GET_PD_IDX((uint64_t)virtAddr), GET_PT_IDX((uint64_t)virtAddr));
    }

    uint64_t resolved_flags = flags;
    if (flags == (uint64_t)-1)
        resolved_flags = PAGE_PRESENT | PAGE_WRITABLE;

    resolved_flags |= PAGE_PRESENT;     // always set present
    resolved_flags &= ~PAGE_HUGE_PAGES; // never a huge page at PT level
    resolved_flags &= ~PAGE_ADDR_MASK;  // zero addr bits, set separately
    if (paging_print_out)
    {
        log_debug(MODULE, "resolved flags=%x", resolved_flags);
    }

    page_map_level_4 *pml4 = (page_map_level_4 *)page_paging->page_dir;
    page_table_entry64 *pdpt_entry = paging64_get_pdpt_entry(page_paging, virt, false);
    if (pdpt_entry && pdpt_entry->present && pdpt_entry->ps_or_pat)
    {
        log_warn(MODULE, "why are you trying to map the kernel?");
        return;
    }
    page_table_entry64 *pd_entry = paging64_get_pd_entry(page_paging, virt, false);
    if (pd_entry && pd_entry->present && pd_entry->ps_or_pat)
    {
        log_warn(MODULE, "why are you trying to map the kernel?");
        return;
    }

    page_table64 *pt = paging64_get_pt(page_paging, virtAddr, 1);
    if (!pt)
    {
        write_error(LVL_ERROR, "Paging", "Failed to allocate page table for virt 0x%llx", (uint64_t)virtAddr);
        return;
    }

    page_table_entry64 *pt_entry = &pt->e[GET_PT_IDX((uint64_t)virtAddr)];

    if (pt_entry->present)
    {
        write_error(LVL_WARN, "Paging", "Remapping already-mapped page at virt 0x%llx", (uint64_t)virtAddr);
    }

    pt_entry->raw = resolved_flags & PAGE_FLAGS_MASK;
    pt_entry->addr = (uint64_t)physAddr >> 12;

    if (paging_print_out)
    {
        log_info(MODULE, "mapped virt=%p -> phys=%p (pdi=%u pti=%u flags=%x)", virtAddr, physAddr, GET_PD_IDX((uint64_t)virtAddr), GET_PT_IDX((uint64_t)virtAddr), resolved_flags);
    }

    // Invalidate the TLB entry for this address
    paging_native_flush_tlb_single(virtAddr);
}

// Unmap a single virtual page.
void paging_unmap_page(paging_page *paging_page, virt_addr virtAddr)
{
    virt_addr virt = virtAddr;
    uint64_t pml4_index = GET_PML4_IDX((uint64_t)virt);
    uint64_t pdpt_index = GET_PDPT_IDX((uint64_t)virt);
    uint64_t pd_index = GET_PD_IDX((uint64_t)virt);
    uint64_t pt_index = GET_PT_IDX((uint64_t)virt);

    // Fetch all four tables (no alloc — if any is missing, nothing to unmap)
    page_map_level_4 *pml4 = paging64_get_pml4(paging_page, virtAddr, 0);
    page_dpt *pdpt = paging64_get_pdpt(paging_page, virtAddr, 0);
    page_directory64 *pd = paging64_get_pd(paging_page, virtAddr, 0);
    page_table64 *pt = paging64_get_pt(paging_page, virtAddr, 0);

    if (!pt)
    {
        return;
    }

    page_table_entry64 *pml4_entry = &pml4->e[pml4_index];
    page_table_entry64 *pdpt_entry = &pdpt->e[pdpt_index];
    page_table_entry64 *pd_entry = &pd->e[pd_index];
    page_table_entry64 *pt_entry = &pt->e[pt_index];

    if (!pt_entry->present)
        return;

    // Grab physical addresses before we zero the entries
    phys_addr pt_phys = (phys_addr)(uint64_t)(pd_entry->addr << 12);
    phys_addr pd_phys = (phys_addr)(uint64_t)(pdpt_entry->addr << 12);
    phys_addr pdpt_phys = (phys_addr)(uint64_t)(pml4_entry->addr << 12);

    pt_entry->raw = 0;

    // Walk back up, freeing empty tables
    if (paging64_table_is_empty(pt->e))
    {
        pd_entry->raw = 0;
        paging_free_frame(pt_phys);

        if (paging64_table_is_empty(pd->e))
        {
            pdpt_entry->raw = 0;
            paging_free_frame(pd_phys);

            if (paging64_table_is_empty(pdpt->e))
            {
                pml4_entry->raw = 0;
                paging_free_frame(pdpt_phys);
                // Never free pml4 itself — that's the root, caller owns it
            }
        }
    }

    paging_native_flush_tlb_single(virtAddr);
}

void paging_print_info(virt_addr cr2)
{
}
