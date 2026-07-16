/*
 * File: paging_64.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#ifdef __x86_64__
#include "paging_64.h"
#include "kernel/string.h"
#include "kernel/memory.h"
#include "task/process.h"
#include "mm/memdefs.h"
#include "mm/pmm.h"

#define GET_PML4_IDX(va) ((((uint64_t)va) >> 39) & 0x1FF)
#define GET_PDPT_IDX(va) ((((uint64_t)va) >> 30) & 0x1FF)
#define GET_PD_IDX(va)   ((((uint64_t)va) >> 21) & 0x1FF)
#define GET_PT_IDX(va)   ((((uint64_t)va) >> 12) & 0x1FF)

#define MODULE           "X86_PAGING_64"

bool paging_print_out = true;

extern uint64_t mm_flags_to_pte(mmu_flags_t flags);
extern mmu_flags_t pte_to_mm_flags(uint64_t pte);

vaddr_t get_next(page_table_entry64 *entry, paging_level level)
{
    if (!entry->present)
    {
        return 0;
    }

    uint64_t next = entry->addr << 12;

    if (entry->ps_or_pat)
    {
        next <<= 9;
        if (level == PAGING_LEVEL_PDPT)
        {
            next <<= 9;
        }
    }

    paddr_t next_addr = (paddr_t)next;
    /*     if (entry->kernel)
        {
            log_debug(MODULE, "get_next using kernel offset from %p", next_addr);
            return paging_phys_to_virt(next_addr);
        }
        log_debug(MODULE, "get_next using heap offset from %p", next_addr); */
    return phys_to_virt_auto(next_addr);
}

void map(page_map_level_4 *pml4_virt, vaddr_t virtAddr, paddr_t physAddr, size_t size)
{
    size_t total_size = ALIGN_2_UP(size, 0x200000);
    log_debug(MODULE, "map %p..%p for %u bytes", physAddr, virtAddr, total_size);
    log_debug(MODULE, "map %p..%p-%p..%p", physAddr, virtAddr, physAddr + total_size, virtAddr + total_size);
    vaddr_t virt = (vaddr_t)ALIGN_2_UP((uint64_t)virtAddr, PAGE_SIZE);
    uint64_t resolved_flags = PAGE_PRESENT | PAGE_WRITABLE;
    page_map_level_4 *pml4 = pml4_virt;
    int pml4_idx = GET_PML4_IDX(virt);
    int pdpt_idx = GET_PDPT_IDX(virt);
    int pd_idx = GET_PD_IDX(virt);
    int pt_idx = GET_PT_IDX(virt);
    log_debug(MODULE, "indices pml4=%u pdpt=%u pd=%u pt=%u", pml4_idx, pdpt_idx, pd_idx, pt_idx);
    if (!pml4)
    {
        return;
    }
#define ALLOCATE_NEW_TABLE(entry)                                                   \
    paddr_t phys = pmm_alloc_frame();                                               \
    vaddr_t v = phys_to_virt_auto(phys);                                            \
    log_debug(MODULE, "alloc frame phys=%p virt=%p for entry@%p", phys, v, &entry); \
    uint64_t *new_entry = (uint64_t *)v;                                            \
    for (size_t i = 0; i < PT64_ENTRIES; i++)                                       \
    {                                                                               \
        new_entry[i] = 0;                                                           \
    }                                                                               \
    entry.raw = 0;                                                                  \
    entry.present = 1;                                                              \
    entry.writable = 1;                                                             \
    entry.addr = (uint64_t)phys >> 12;                                              \
    log_debug(MODULE, "entry.raw = 0x%llx @ %p", entry.raw, &entry);

    if (!pml4->e[pml4_idx].present)
    {
        log_debug(MODULE, "allocating PDPT for pml4[%u]", pml4_idx);
        ALLOCATE_NEW_TABLE(pml4->e[pml4_idx]);
    }
    else
    {
        log_debug(MODULE, "pml4[%u] already present raw=0x%llx", pml4_idx, pml4->e[pml4_idx].raw);
    }

    vaddr_t pdpt_virt = get_next(&pml4->e[pml4_idx], PAGING_LEVEL_PML4);
    log_debug(MODULE, "pdpt_virt = %p", pdpt_virt);
    page_dpt *pdpt = (page_dpt *)pdpt_virt;
    if (!pdpt)
    {
        log_err(MODULE, "pdpt is null, aborting");
        return;
    }

    if (!pdpt->e[pdpt_idx].present)
    {
        log_debug(MODULE, "allocating PD for pdpt[%u]", pdpt_idx);
        ALLOCATE_NEW_TABLE(pdpt->e[pdpt_idx]);
    }
    else
    {
        log_debug(MODULE, "pdpt[%u] already present raw=0x%llx", pdpt_idx, pdpt->e[pdpt_idx].raw);
    }

    int pt_count = total_size / PAGE_SIZE;
    int pd_count = pt_count / PT64_ENTRIES;
    vaddr_t pd_virt = get_next(&pdpt->e[pdpt_idx], PAGING_LEVEL_PDPT);
    log_debug(MODULE, "pt_count = %u, pd_count = %u, pd_virt = %p", pt_count, pd_count, pd_virt);
    page_directory64 *pd = (page_directory64 *)pd_virt;
    if (!pd)
    {
        log_err(MODULE, "pd is null, aborting");
        return;
    }

    int frame_index = 0;
    for (size_t i = 0; i < pd_count; i++)
    {
        if (!pd->e[pd_idx + i].present)
        {
            if (pd_count < 500)
            {
                paddr_t phys = pmm_alloc_frame();
                vaddr_t v = phys_to_virt_auto(phys);
                log_debug(MODULE, "4K path: pd[%u] PT phys=%p virt=%p", pd_idx + i, phys, v);
                uint64_t *new_entry = (uint64_t *)v;
                for (size_t i = 0; i < 512; i++)
                {
                    new_entry[i] = 0;
                }
                pd->e[pd_idx + i].raw = 0;
                pd->e[pd_idx + i].present = 1;
                pd->e[pd_idx + i].writable = 1;
                pd->e[pd_idx + i].addr = (uint64_t)phys >> 12;
                page_table64 *pt = (page_table64 *)new_entry;
                for (size_t k = 0; k < PT64_ENTRIES; k++)
                {
                    uint64_t target_index = (uint64_t)frame_index + ((uint64_t)physAddr >> 12);
                    log_debug(MODULE, "pml4[%d]->pdpt[%d]->pd[%u]->pt[%u] mapping p%p..v%p", pml4_idx, pdpt_idx, pd_idx + i, k, (uint64_t)target_index << 12, virt + frame_index * PAGE_SIZE);
                    pt->e[k].raw = 0;
                    pt->e[k].present = 1;
                    pt->e[k].writable = 1;
                    pt->e[k].addr = target_index;
                    frame_index++;
                }
            }
            else
            {
                paddr_t huge_phys = (paddr_t)((uint64_t)physAddr + i * 0x200000);
                log_debug(MODULE, "2MiB path: pd[%u] phys=%p virt=%p", pd_idx + i, huge_phys, virt + i * 0x200000);
                pd_huge_entry64 *huge_pd = (pd_huge_entry64 *)(void *)(&pd->e[pd_idx + i]);
                huge_pd->raw = 0;
                huge_pd->present = 1;
                huge_pd->writable = 1;
                huge_pd->ps = 1;
                huge_pd->addr = (uint64_t)huge_phys >> 21;
                log_debug(MODULE, "pml4[%d]->pdpt[%d]->pd[%u] mapping huge entry raw=0x%llx addr_field=%llx", pml4_idx, pdpt_idx, pd_idx + i, huge_pd->raw, huge_pd->addr);
            }
        }
        else
        {
            log_debug(MODULE, "pd[%u] already present, skipping", pd_idx + i);
        }
    }
#undef ALLOCATE_NEW_TABLE
    log_info(MODULE, "mapped virt=%016llx -> phys=%016llx (pml4i=%u pdpti=%u pdi=%u pti=%u flags=%x)",
             virtAddr, physAddr,
             GET_PML4_IDX((uint64_t)virtAddr), GET_PDPT_IDX((uint64_t)virtAddr),
             GET_PD_IDX((uint64_t)virtAddr), GET_PT_IDX((uint64_t)virtAddr), resolved_flags);
}

void paging_init64(page_table_t *kernel_page, vaddr_t kernel_addr, size_t kernel_size, paddr_t pmm_start, paddr_t pmm_end)
{
    paddr_t kernel_pml4 = pmm_alloc_frame();
    paddr_t kernel_pdpt = pmm_alloc_frame();

    log_debug(MODULE, "kernel_pml4 = %p", kernel_pml4);

    page_map_level_4 *kernel_pml4_virt = (page_map_level_4 *)phys_to_virt_auto(kernel_pml4);
    page_dpt *kernel_pdpt_virt = (page_dpt *)phys_to_virt_auto(kernel_pdpt);

    uint64_t kernel_start_phys = (uint64_t)(kernel_addr - (KERNEL_VIRT_BASE - KERNEL_PHYS_BASE));
    log_debug(MODULE, "kernel_addr = %p kernel_start_phys = %p", kernel_addr, kernel_start_phys);

    log_debug(MODULE, "zeroing %u entries from pml4 %p and pdpt %p", PT64_ENTRIES, kernel_pml4_virt, kernel_pdpt_virt);
    for (size_t i = 0; i < PT64_ENTRIES; i++)
    {
        kernel_pml4_virt->e[i].raw = 0;
        kernel_pdpt_virt->e[i].raw = 0;
    }
    log_debug(MODULE, "done zeroing %u entries", PT64_ENTRIES);

    map(kernel_pml4_virt, (vaddr_t)kernel_addr, (paddr_t)kernel_start_phys, kernel_size);
    paddr_t start_phys_aligned = pmm_start; // pmm_start gotten from the pmm allocator (buddy)
    paddr_t end_phys_aligned = pmm_end;     // pmm_end gotten from the pmm allocator (buddy)
    size_t pmm_size = ALIGN_2_UP(end_phys_aligned - start_phys_aligned, 0x1000000);
    log_warn(MODULE, "pmm_start = %p pmm_end = %p", pmm_start, pmm_end);
    map(kernel_pml4_virt, (vaddr_t)MEMORY_DIRECT_MAP_VIRT_BASE, start_phys_aligned, pmm_size);

    kernel_page->page_dir = (vaddr_t)kernel_pml4_virt;
    kernel_page->page_dir_phys = kernel_pml4;
    log_debug(MODULE, "setting cr3");
    __asm__ volatile(
        "mov cr3, %0" ::"r"((uint64_t)kernel_pml4) : "rax");
    log_debug(MODULE, "kernel_page @%p", kernel_page);
    mmu_arch_flush_all();
}

/* ===== HELPER FUNCTIONS ===== */
static int paging64_table_is_empty(page_table_entry64 *entries)
{
    for (int i = 0; i < 512; i++)
    {
        if (entries[i].present)
        {
            return 0;
        }
    }
    return 1;
}

// Returns a pointer to the entry at the requested level, or NULL on failure.
// If alloc=1, missing intermediate tables are allocated and zeroed.
// If alloc=0, returns NULL if any intermediate table is missing.
page_table_entry64 *paging64_get_entry(page_table_t *page_table, vaddr_t virtAddr, paging_level target_level, bool alloc, bool user)
{
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virtAddr),
        GET_PDPT_IDX((uint64_t)virtAddr),
        GET_PD_IDX((uint64_t)virtAddr),
        GET_PT_IDX((uint64_t)virtAddr),
    };

    page_map_level_4 *pml4 = (page_map_level_4 *)page_table->page_dir;
    page_table_entry64 *table = pml4->e;
    if (paging_print_out)
    {
        log_debug(MODULE, "table at %p", table);
    }

    for (int level = 4; level > (int)target_level; level--)
    {
        if (paging_print_out)
        {
            log_debug(MODULE, "table[%u] at %p", idx[4 - level], &table[idx[4 - level]]);
        }
        page_table_entry64 *entry = &table[idx[4 - level]];
        if (paging_print_out)
        {
            log_debug(MODULE, "at level %u(%u): entry raw = 0x%016llx", level, idx[4 - level], entry->raw);
            log_debug(MODULE, "addr = p%p, flags = 0x%x", entry->addr << 12, entry->raw & PAGE_FLAGS_MASK);
        }
        if (!entry->present)
        {
            if (!alloc)
            {
                return NULL;
            }

            if (paging_print_out)
            {
                log_debug(MODULE, "allocating new at level %u at %u", level, idx[4 - level]);
            }

            paddr_t new_phys = pmm_alloc_frame();
            if (paging_print_out)
            {
                log_debug(MODULE, "got frame %p", new_phys);
            }
            if (!new_phys)
            {
                return NULL;
            }

            vaddr_t new_virt = phys_to_virt_auto(new_phys);
            log_debug("TEST", "memset(%p, 0, 4096)", new_virt);
            memset((void *)new_virt, 0, PAGE_SIZE);

            entry->raw = 0;
            uint64_t addr = (uint64_t)new_phys >> 12;
            if (entry->ps_or_pat)
            {
                addr >>= 9;
                if (level == PAGING_LEVEL_PDPT)
                {
                    log_err(MODULE, "entry is a huge PDPT");
                    addr >>= 9;
                    pdpt_huge_entry64 *huge_entry = (pdpt_huge_entry64 *)entry;
                    huge_entry->addr = addr;
                }
                else
                {
                    log_err(MODULE, "entry is a huge PD");
                    pd_huge_entry64 *huge_entry = (pd_huge_entry64 *)entry;
                    huge_entry->addr = addr;
                }
            }
            else
            {
                entry->addr = addr;
            }
            entry->present = 1;
            entry->writable = 1;
            entry->user = user;
        }

        paddr_t next_phys = (paddr_t)((uint64_t)entry->addr << 12);
        page_map_level_4 *next_table_entry = ((page_map_level_4 *)get_next(entry, level));
        if (paging_print_out)
        {
            log_debug(MODULE, "next level table at v%p/p%p", next_table_entry, next_phys);
        }
        table = next_table_entry->e;
    }

    page_table_entry64 *result = &table[idx[4 - target_level]];
    if (paging_print_out)
    {
        log_debug(MODULE, "result level %u: entry raw = 0x%016llx", target_level, result->raw);
        log_debug(MODULE, "addr = p%p, flags = 0x%x", result->addr, result->raw & PAGE_FLAGS_MASK);
    }
    return result;
}

// Returns a pointer to the table AT the requested level, or NULL on failure.
// e.g. PAGING_LEVEL_PT returns the page_table64* that contains the PTE for virtAddr.
// If alloc=1, missing intermediate tables are allocated and zeroed.
void *paging64_get_table(page_table_t *page_table, vaddr_t virtAddr, paging_level target_level, int alloc, bool user)
{
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virtAddr),
        GET_PDPT_IDX((uint64_t)virtAddr),
        GET_PD_IDX((uint64_t)virtAddr),
        GET_PT_IDX((uint64_t)virtAddr),
    };

    page_map_level_4 *pml4 = (page_map_level_4 *)page_table->page_dir;
    page_table_entry64 *table = pml4->e;
    if (paging_print_out)
    {
        log_debug(MODULE, "table at %p", table);
    }

    for (int level = 4; level >= (int)target_level; level--)
    {
        if (level == (int)target_level)
        {
            if (paging_print_out)
            {
                log_debug(MODULE, "result level %u at %p", target_level, table);
            }
            return (void *)table;
            // cast to page_map_level_4/page_directory64/etc. at call site
        }
        if (paging_print_out)
        {
            log_debug(MODULE, "table[%u] at %p", idx[4 - level], &table[idx[4 - level]]);
        }
        page_table_entry64 *entry = &table[idx[4 - level]];
        if (paging_print_out)
        {
            log_debug(MODULE, "at level %u(%u): entry raw = 0x%016llx entry@v%p", level, idx[4 - level], entry->raw, entry);
            log_debug(MODULE, "addr = phys%p, flags = 0x%x", entry->addr << 12, entry->raw & PAGE_FLAGS_MASK);
        }

        if (!entry->present)
        {
            if (!alloc)
            {
                return NULL;
            }

            if (paging_print_out)
            {
                log_debug(MODULE, "allocating new at level %u at index %u", level, idx[4 - level]);
            }

            paddr_t new_phys = pmm_alloc_frame();
            if (!new_phys)
            {
                return NULL;
            }

            vaddr_t new_virt = phys_to_virt_auto(new_phys);

            memset((void *)new_virt, 0, PAGE_SIZE);

            if (paging_print_out)
            {
                log_debug(MODULE, "setting the fields on %p", entry);
                entry->raw = 0xFF00FF00;
                if (entry->raw != 0xFF00FF00)
                {
                    log_err(MODULE, "entry can't be written too");
                }
                log_debug(MODULE, "setting the fields on %p@%llx", entry, entry->raw);
            }

            entry->raw = 0;
            uint64_t addr = (uint64_t)new_phys >> 12;
            if (entry->ps_or_pat)
            {
                addr >>= 9;
                if (level == PAGING_LEVEL_PDPT)
                {
                    log_err(MODULE, "entry is a huge PDPT");
                    addr >>= 9;
                    pdpt_huge_entry64 *huge_entry = (pdpt_huge_entry64 *)entry;
                    huge_entry->addr = addr;
                }
                else
                {
                    log_err(MODULE, "entry is a huge PD");
                    pd_huge_entry64 *huge_entry = (pd_huge_entry64 *)entry;
                    huge_entry->addr = addr;
                }
            }
            else
            {
                entry->addr = addr;
            }
            entry->present = 1;
            entry->writable = 1;
            entry->user = user;
            if (paging_print_out)
            {
                log_debug(MODULE, "at level %u(%u): entry raw = 0x%016llx entry@v%p", level, idx[4 - level], entry->raw, entry);
                log_debug(MODULE, "addr = p%p, flags = 0x%x", entry->addr << 12, entry->raw & PAGE_FLAGS_MASK);
            }
        }

        paddr_t next_phys = (paddr_t)((uint64_t)entry->addr << 12);
        page_map_level_4 *next_table_entry = ((page_map_level_4 *)get_next(entry, level));
        if (paging_print_out)
        {
            log_debug(MODULE, "next level table at v%p/p%p", next_table_entry, next_phys, entry->addr);
        }
        table = next_table_entry->e;
    }

    return NULL; // unreachable
}

page_map_level_4 *paging64_get_pml4(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pml4 table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_map_level_4 *)paging64_get_table(p, v, PAGING_LEVEL_PML4, alloc, user);
}

page_dpt *paging64_get_pdpt(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pdpt table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_dpt *)paging64_get_table(p, v, PAGING_LEVEL_PDPT, alloc, user);
}

page_directory64 *paging64_get_pd(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pd table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_directory64 *)paging64_get_table(p, v, PAGING_LEVEL_PD, alloc, user);
}

page_table64 *paging64_get_pt(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pt table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table64 *)paging64_get_table(p, v, PAGING_LEVEL_PT, alloc, user);
}

page_table_entry64 *paging64_get_pml4_entry(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pml4 entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PML4, alloc, user);
}

page_table_entry64 *paging64_get_pdpt_entry(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pdpt entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PDPT, alloc, user);
}

page_table_entry64 *paging64_get_pd_entry(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pd entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PD, alloc, user);
}

page_table_entry64 *paging64_get_pt_entry(page_table_t *p, vaddr_t v, int alloc, bool user)
{
    if (paging_print_out)
    {
        log_info(MODULE, "finding and try allocate (%s) pt entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PT, alloc, user);
}

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
paddr_t paging_get_physical(page_table_t *page_table, vaddr_t virt)
{
    page_map_level_4 *pml4 = (page_map_level_4 *)page_table->page_dir;

    int pml4_idx = GET_PML4_IDX(virt);

    if (!pml4 || pml4->e[pml4_idx].present == 0)
    {
        return 0;
    }

    page_table_entry64 *pdpt = (page_table_entry64 *)paging64_get_pdpt_entry(page_table, virt, false, false);
    if (!pdpt || pdpt->present == 0)
    {
        return 0;
    }

    // 1 GiB huge page
    if (pdpt->ps_or_pat)
    {
        return (paddr_t)((pdpt->addr << 30) | (virt & 0x3FFFFFFF));
    }

    page_table_entry64 *pd = (page_table_entry64 *)paging64_get_pd_entry(page_table, virt, false, false);
    if (!pd || pd->present == 0)
    {
        return 0;
    }

    // 2 MiB huge page
    if (pd->ps_or_pat)
    {
        return (paddr_t)((pd->addr << 21) | (virt & 0x1FFFFF));
    }

    page_table_entry64 *pt = (page_table_entry64 *)paging64_get_pt_entry(page_table, virt, false, false);
    if (!pt || pt->present == 0)
    {
        return 0;
    }

    // 4 KiB page
    return (paddr_t)((pt->addr << 12) | (virt & 0xFFF));
}

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
vaddr_t paging_get_virtual(page_table_t *page_table, paddr_t physAddr)
{
    uint64_t phys = (uint64_t)physAddr & ~0xFFF;
    page_map_level_4 *pml4 = (page_map_level_4 *)page_table->page_dir;

    for (uint32_64 pml4i = 0; pml4i < PT64_ENTRIES; pml4i++)
    {
        page_table_entry64 *pml4e = &pml4->e[pml4i];
        // log_debug(MODULE, "pml4[%u] = %llx", pml4i, pml4e->raw);
        if (!pml4e->present)
        {
            continue;
        }

        page_dpt *pdpt = (page_dpt *)phys_to_virt_auto((paddr_t)(pml4e->addr << 12));

        for (uint32_64 pdpti = 0; pdpti < PT64_ENTRIES; pdpti++)
        {
            page_table_entry64 *pdpte = &pdpt->e[pdpti];
            // log_debug(MODULE, "pdpt[%u] = %llx", pdpti, pdpte->raw);
            if (!pdpte->present)
            {
                continue;
            }

            // 1GiB huge page (PS=1)
            if (pdpte->ps_or_pat)
            {
                paddr_t page_phys = (paddr_t)pdpte->addr << 30;
                if ((phys & ~0x3FFFFFFFull) == page_phys)
                {
                    vaddr_t va = (vaddr_t)((pml4i << 39) |
                                           (pdpti << 30) |
                                           (phys & 0x3FFFFFFF));
                    return (vaddr_t)va;
                }
                continue;
            }

            page_directory64 *pd = (page_directory64 *)phys_to_virt_auto((paddr_t)(pdpte->addr << 12));

            for (uint16_t pdi = 0; pdi < PT64_ENTRIES; pdi++)
            {
                page_table_entry64 *pde = &pd->e[pdi];
                if (!pde->present)
                {
                    continue;
                }

                // 2MiB huge page (PS=1)
                if (pde->ps_or_pat)
                {
                    paddr_t page_phys = (paddr_t)pde->addr << 21;
                    if ((phys & ~0x1FFFFFull) == page_phys)
                    {
                        vaddr_t va = (vaddr_t)(((paddr_t)pml4i << 39) |
                                               ((paddr_t)pdpti << 30) |
                                               ((paddr_t)pdi << 21) |
                                               (phys & 0x1FFFFF));
                        return (vaddr_t)va;
                    }
                    continue;
                }

                page_table64 *pt = (page_table64 *)phys_to_virt_auto((paddr_t)(pde->addr << 12));

                for (uint16_t pti = 0; pti < PT64_ENTRIES; pti++)
                {
                    page_table_entry64 *pte = &pt->e[pti];
                    if (!pte->present)
                    {
                        continue;
                    }

                    if (((paddr_t)pte->addr << 12) == phys)
                    {
                        vaddr_t va = (vaddr_t)((pml4i << 39) |
                                               (pdpti << 30) |
                                               (pdi << 21) |
                                               (pti << 12));
                        log_debug(MODULE, "Done got pml4i %u pdpti %u pdi %u pti %u from p0x%llx",
                                  pml4i, pdpti, pdi, pti, physAddr);
                        return (vaddr_t)va;
                    }
                }
            }
        }
    }

    // log_warn("Paging", "could not get virtual from %p", physAddr);
    return 0;
}

// Map a single virtual page → physical frame in the current page directory.
int paging_map_page(page_table_t *page_table, vaddr_t virtAddr, paddr_t physAddr, mmu_flags_t flags)
{
    vaddr_t virt = (vaddr_t)ALIGN_2_UP((uint64_t)virtAddr, PAGE_SIZE);

    int pml4_idx = GET_PML4_IDX(virt);
    int pdpt_idx = GET_PDPT_IDX(virt);
    int pd_idx = GET_PD_IDX(virt);
    int pt_idx = GET_PT_IDX(virt);

    uint64_t resolved_flags = mm_flags_to_pte(flags) & 0x0FF;
    uint64_t leaf_resolved_flags = mm_flags_to_pte(flags);

    if (paging_print_out)
    {
        log_debug(MODULE, "resolved flags=%llx", resolved_flags);
    }

    log_info(MODULE, "map_page virt=%p phys=%p flags=%lx leaf_flags=%lx pml4[%u]->pdpt[%u]->pd[%u]->pt[%u] to %p", virt, physAddr, resolved_flags, leaf_resolved_flags, pml4_idx, pdpt_idx, pd_idx, pt_idx, page_table->page_dir);

    bool user_flag = ((resolved_flags & PAGE_USER) == PAGE_USER);
    page_map_level_4 *pml4 = paging64_get_pml4(page_table, virtAddr, 1, user_flag);
    page_dpt *pdpt = paging64_get_pdpt(page_table, virtAddr, 1, user_flag);
    page_directory64 *pd = paging64_get_pd(page_table, virtAddr, 1, user_flag);
    page_table64 *pt = paging64_get_pt(page_table, virtAddr, 1, user_flag);

    page_table_entry64 *pml4e = &pml4->e[pml4_idx];
    page_table_entry64 *pdpte = &pdpt->e[pdpt_idx];
    page_table_entry64 *pde = &pd->e[pd_idx];
    page_table_entry64 *pte = &pt->e[pt_idx];

#define PRINT(table, entry)                                                                                                                         \
    if (!table || !entry)                                                                                                                           \
    {                                                                                                                                               \
        log_err(MODULE, "%s[%u] %p is null", #table, table##_idx, entry);                                                                           \
        return RETURN_ERROR;                                                                                                                        \
    }                                                                                                                                               \
    log_debug(MODULE, "%s @ %p = 0x%lx { addr = %p, flags = 0x%llx }", #entry, entry, entry->raw, entry->addr << 12, entry->raw & PAGE_FLAGS_MASK); \
    if (paging_print_out)                                                                                                                           \
    {                                                                                                                                               \
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", entry->addr << 12, entry->raw & PAGE_FLAGS_MASK);                                           \
    }

#define PRINT_INT(table, entry)                        \
    PRINT(table, entry)                                \
    {                                                  \
        uint64_t addr = entry->addr;                   \
        entry->raw = resolved_flags & PAGE_FLAGS_MASK; \
        entry->addr = addr;                            \
    }

#define PRINT_HP(table, entry)                                                                  \
    PRINT_INT(table, entry)                                                                     \
    if (entry->ps_or_pat)                                                                       \
    {                                                                                           \
        log_err(MODULE, "%s[%u] %p is huge raw = %lx", #table, table##_idx, entry, entry->raw); \
        return RETURN_ERROR;                                                                    \
    }

    PRINT_INT(pml4, pml4e);

    PRINT_HP(pdpt, pdpte);

    PRINT_HP(pd, pde);

    if (!pt || !pte)
    {
        log_err(MODULE, "pt is null");
        return RETURN_ERROR;
    }

    if (pte->present)
    {
        log_err(MODULE, "Failed to allocate page table for virt 0x%llx", virtAddr);
        log_err(MODULE, "Remapping already-mapped page at virt 0x%llx", virtAddr);
        log_err(MODULE, "pt[%u] is already present @ %p", pt_idx, pte);
        return RETURN_ERROR;
    }

    if (paging_print_out)
    {
        log_debug(MODULE, "got entry at v%p from table %p", pte, pt);
    }

    pte->raw = leaf_resolved_flags & PAGE_FLAGS_MASK;
    pte->addr = (uint64_t)physAddr >> 12;
    PRINT(pt, pte);

    if (paging_print_out)
    {
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pml4e->addr << 12, pml4e->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pdpte->addr << 12, pdpte->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pde->addr << 12, pde->raw & PAGE_FLAGS_MASK);

        log_debug(MODULE, "PT[%u] readback = 0x%llx", pt_idx, pte->raw);
    }

    if (paging_print_out)
    {
        log_debug(MODULE, "pte = 0x%llx/0b%064b", pte->raw, pte->raw);
    }
    log_info(MODULE, "mapped virt=%p -> phys=%p (pml4i=%u pdpti=%u pdi=%u pti=%u flags=%llx)", virtAddr, physAddr, pml4_idx, pdpt_idx, pd_idx, pt_idx, resolved_flags);

    // Invalidate the TLB entry for this address
    mmu_arch_flush_page(virtAddr);
    return RETURN_GOOD;
#undef ALLOCATE_NEW_TABLE
}

void paging_clean_up(page_table_t *page_table, vaddr_t virtAddr)
{
    vaddr_t virt = virtAddr;
    uint64_t pml4_index = GET_PML4_IDX(virt);
    uint64_t pdpt_index = GET_PDPT_IDX(virt);
    uint64_t pd_index = GET_PD_IDX(virt);
    uint64_t pt_index = GET_PT_IDX(virt);

    log_debug(MODULE, "paging64_clean_up: virt=0x%016llx [pml4=%llu pdpt=%llu pd=%llu pt=%llu]",
              virtAddr, pml4_index, pdpt_index, pd_index, pt_index);

    // Fetch all four tables (no alloc — if any is missing, nothing to unmap)
    page_map_level_4 *pml4 = paging64_get_pml4(page_table, virtAddr, 0, false);
    page_dpt *pdpt = paging64_get_pdpt(page_table, virtAddr, 0, false);
    page_directory64 *pd = paging64_get_pd(page_table, virtAddr, 0, false);
    page_table64 *pt = paging64_get_pt(page_table, virtAddr, 0, false);

    if (!pt)
    {
        log_err(MODULE, "paging64_clean_up: no PT for virt=0x%016llx, nothing to clean up", virtAddr);
        return;
    }

    page_table_entry64 *pml4_entry = &pml4->e[pml4_index];
    page_table_entry64 *pdpt_entry = &pdpt->e[pdpt_index];
    page_table_entry64 *pd_entry = &pd->e[pd_index];

    if (!pt->e[pt_index].present)
    {
        return;
    }

    // Grab physical addresses before we zero the entries
    paddr_t pt_phys = (paddr_t)(pd_entry->addr << 12);
    paddr_t pd_phys = (paddr_t)(pdpt_entry->addr << 12);
    paddr_t pdpt_phys = (paddr_t)(pml4_entry->addr << 12);

    // Walk back up, freeing empty tables
    if (paging64_table_is_empty(pt->e))
    {
        log_debug(MODULE, "paging64_clean_up: PT empty, freeing PT phys=0x%016llx", pt_phys);
        pd_entry->raw = 0;
        pmm_free_frame(pt_phys);

        if (paging64_table_is_empty(pd->e))
        {
            log_debug(MODULE, "paging64_clean_up: PD empty, freeing PD phys=0x%016llx", pd_phys);
            pdpt_entry->raw = 0;
            pmm_free_frame(pd_phys);

            if (paging64_table_is_empty(pdpt->e))
            {
                log_debug(MODULE, "paging64_clean_up: PDPT empty, freeing PDPT phys=0x%016llx", pdpt_phys);
                pml4_entry->raw = 0;
                pmm_free_frame(pdpt_phys);
                // Never free pml4 itself — that's the root, caller owns it
            }
        }
    }

    mmu_arch_flush_page(virtAddr);
    log_debug(MODULE, "paging64_clean_up: TLB flushed for virt=0x%016llx", virtAddr);
}

// Unmap a single virtual page.
paddr_t paging_unmap_page(page_table_t *page_table, vaddr_t virtAddr)
{
    vaddr_t virt = virtAddr;
    uint64_t pml4_index = GET_PML4_IDX(virt);
    uint64_t pdpt_index = GET_PDPT_IDX(virt);
    uint64_t pd_index = GET_PD_IDX(virt);
    uint64_t pt_index = GET_PT_IDX(virt);

    log_debug(MODULE, "paging_unmap_page: virt=0x%016llx [pml4=%llu pdpt=%llu pd=%llu pt=%llu]",
              virtAddr, pml4_index, pdpt_index, pd_index, pt_index);

    // Fetch all four tables (no alloc — if any is missing, nothing to unmap)
    page_table64 *pt = paging64_get_pt(page_table, virtAddr, 0, false);

    if (!pt)
    {
        log_err(MODULE, "paging_unmap_page: no PT for virt=0x%016llx, nothing to unmap", virtAddr);
        return RETURN_FAILED;
    }

    page_table_entry64 *pt_entry = &pt->e[pt_index];

    if (!pt_entry->present)
    {
        log_err(MODULE, "paging_unmap_page: PTE not present for virt=0x%016llx, skipping", virtAddr);
        return RETURN_FAILED;
    }

    log_info(MODULE, "paging_unmap_page: unmapping virt=0x%016llx", virtAddr);

    paddr_t result = (paddr_t)((pt_entry->addr << 12) | (virt & 0xFFF));

    pt_entry->raw = 0;

    mmu_arch_flush_page(virtAddr);
    log_debug(MODULE, "paging_unmap_page: TLB flushed for virt=0x%016llx", virtAddr);
    return result;
}

static spinlock_t page_copy_lock = {0};

void page_copy(paddr_t src, paddr_t dst)
{
    spinlock_acquire(&page_copy_lock);

    // map src and dst into scratch windows
    paging_map_page(&kernel_page, MEMORY_PHYS_COPY_SRC, src, kernel_data_flags);
    paging_map_page(&kernel_page, MEMORY_PHYS_COPY_DST, dst, kernel_data_flags);

    // flush TLB for both
    mmu_arch_flush_page(MEMORY_PHYS_COPY_SRC);
    mmu_arch_flush_page(MEMORY_PHYS_COPY_DST);

    memcpy((void *)MEMORY_PHYS_COPY_DST, (void *)MEMORY_PHYS_COPY_SRC, PAGE_SIZE);

    // unmap scratch windows
    paging_unmap_page(&kernel_page, MEMORY_PHYS_COPY_SRC);
    paging_unmap_page(&kernel_page, MEMORY_PHYS_COPY_DST);

    spinlock_release(&page_copy_lock);
}

paddr_t paging_print_info(page_table_t *page_dir, vaddr_t cr2)
{
    uint64_t PML4 = GET_PML4_IDX(cr2);
    uint64_t PDPT = GET_PDPT_IDX(cr2);
    uint64_t PD = GET_PD_IDX(cr2);
    uint64_t PT = GET_PT_IDX(cr2);
    fprintf(VFS_FD_DEBUG, "cr3->PML4[%u]->PDPT[%u]->PD[%u]->PT[%u]\n", PML4, PDPT, PD, PT);

    page_map_level_4 *pml4 = paging64_get_pml4(page_dir, cr2, 0, 0);
    page_dpt *pdpt = paging64_get_pdpt(page_dir, cr2, 0, 0);
    page_directory64 *pd = paging64_get_pd(page_dir, cr2, 0, 0);
    page_table64 *pt = paging64_get_pt(page_dir, cr2, 0, 0);
    page_table_entry64 *pml4_entry = &pml4->e[PML4];
    page_table_entry64 *pdpt_entry = &pdpt->e[PDPT];
    page_table_entry64 *pd_entry = &pd->e[PD];
    page_table_entry64 *pt_entry = &pt->e[PT];
    fprintf(VFS_FD_DEBUG, "%p PML4[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx\n", pml4, PML4, pml4_entry->raw, pml4_entry->addr << 12, pml4_entry->raw & PAGE_FLAGS_MASK);
    if (!pml4_entry || pml4_entry->present == 0)
    {
        fprintf(VFS_FD_DEBUG, "The mapping is fucked\n");
        return 0;
    }
    fprintf(VFS_FD_DEBUG, "%p PDPT[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx\n", pdpt, PDPT, pdpt_entry->raw, pdpt_entry->addr << 12, pdpt_entry->raw & PAGE_FLAGS_MASK);
    if (!pdpt_entry || pdpt_entry->present == 0)
    {
        fprintf(VFS_FD_DEBUG, "The mapping is fucked\n");
        return 0;
    }

    // 1 GiB huge page
    if (pdpt_entry->ps_or_pat)
    {
        fprintf(VFS_FD_DEBUG, "%p PDPT[%u] is huge\n", pdpt, PDPT);
        return (paddr_t)((pdpt_entry->addr << 30) | (cr2 & 0x3FFFFFFF));
    }

    fprintf(VFS_FD_DEBUG, "%p PD[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx\n", pd, PD, pd_entry->raw, pd_entry->addr << 12, pd_entry->raw & PAGE_FLAGS_MASK);
    if (!pd_entry || pd_entry->present == 0)
    {
        fprintf(VFS_FD_DEBUG, "The mapping is fucked\n");
        return 0;
    }

    // 2 MiB huge page
    if (pd_entry->ps_or_pat)
    {
        fprintf(VFS_FD_DEBUG, "%p PD[%u] is huge\n", pd, PD);
        return (paddr_t)((pd_entry->addr << 21) | (cr2 & 0x1FFFFF));
    }

    fprintf(VFS_FD_DEBUG, "%p PT[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx\n", pt, PT, pt_entry->raw, pt_entry->addr << 12, pt_entry->raw & PAGE_FLAGS_MASK);
    if (!pt_entry || pt_entry->present == 0)
    {
        fprintf(VFS_FD_DEBUG, "The mapping is fucked\n");
        return 0;
    }

    // 4 KiB page
    return (paddr_t)((pt_entry->addr << 12) | (cr2 & 0xFFF));
}
#endif
