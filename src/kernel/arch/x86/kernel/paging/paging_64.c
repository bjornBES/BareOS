/*
 * File: paging_64.c
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#ifdef __x86_64__

#include "paging_64.h"
#include "paging.h"

#include "asm/mmu_arch.h"

#include "mm/pmm/pmm.h"
#include "mm/mmu.h"

#include "debug/debug.h"

#include "panic.h"

#include "align.h"
#include "string.h"
#include "memory.h"

#include <defs.h>

#define MODULE           "x86-paging64"

#define GET_PML4_IDX(va) ((((uint64_t)va) >> 39) & 0x1FF)
#define GET_PDPT_IDX(va) ((((uint64_t)va) >> 30) & 0x1FF)
#define GET_PD_IDX(va)   ((((uint64_t)va) >> 21) & 0x1FF)
#define GET_PT_IDX(va)   ((((uint64_t)va) >> 12) & 0x1FF)

typedef enum
{
    PAGING_LEVEL_PML4 = 4,
    PAGING_LEVEL_PDPT = 3,
    PAGING_LEVEL_PD = 2,
    PAGING_LEVEL_PT = 1,
} paging_level;

paging_flags mm_flags_to_pte(mmu_flags_t flags)
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
        // FLAG_SET(pte, PAGE_NO_EXEC);
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
    if (flags.large)
    {
        FLAG_SET(pte, PAGE_HUGE_PAGES);
    }

    return pte;
}

mmu_flags_t pte_to_mm_flags(paging_flags pte)
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

INTERNAL INLINE paddr_t read_cr3(void)
{
    paddr_t val;
    inline_asm("mov %0, cr3" : "=r"(val));
    return val;
}

INTERNAL INLINE void assert_physmap_roundtrip(const char *where, paddr_t phys)
{
    vaddr_t v = phys_to_virt_auto(phys);
    paddr_t back = virt_to_phys_auto(v);
    if (back != phys)
    {
        log_err(MODULE, "PHYSMAP MISMATCH %s: phys=%p -> virt=%p -> phys=%p (expected %p)", where, phys, v, back, phys);
    }
}

INLINE vaddr_t get_next(page_table_entry64 *entry, paging_level level)
{
    if (!paging_disable_print)
    {
        ENTER_FUNC("%p, %u", entry, level);
    }
    if (!entry->present)
    {
        return 0;
    }

    // log_debug(MODULE, "entry raw = 0x%llx", entry->raw);

    uint64_t next = entry->addr << 12;

    if (entry->ps)
    {
        next <<= 9;
        if (level == PAGING_LEVEL_PDPT)
        {
            next <<= 9;
        }
    }

    paddr_t next_addr = (paddr_t)next;
    assert_physmap_roundtrip("get_next", next_addr);

    vaddr_t result = phys_to_virt_auto(next_addr);
    if (!paging_disable_print)
    {
        log_debug(MODULE, "get_next: phys=%p -> virt=%p (level=%u)", next_addr, result, level);
    }
    return result;
}

INLINE void paging64_make_entry(page_table_entry64 *entry, paddr_t addr, paging_level level, mmu_flags_t flags)
{
    entry->raw = 0;
    if (level != PAGING_LEVEL_PT)
    {
        entry->raw = mm_flags_to_pte(flags) & 0xF;
    }
    else
    {
        flags.global = 0;
        entry->raw = mm_flags_to_pte(flags);
    }

    entry->addr = (addr >> 12);
    log_debug(MODULE, "in paging64_make_entry %03lx-%010lx-%03lx", (entry->raw >> 52) & 0xFFF, entry->addr, entry->raw & 0xFFF);
}

// Returns a pointer to the table AT the requested level, or NULL on failure.
// e.g. PAGING_LEVEL_PT returns the page_table64* that contains the PTE for virt_addr.
// If alloc=1, missing intermediate tables are allocated and zeroed.
void *paging64_get_table(page_table_t *page_table, vaddr_t virt_addr, paging_level target_level, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        ENTER_FUNC("%p, v%p, %u, %s, 0x%x", page_table, virt_addr, target_level, alloc BOOL_TO_STRING, flags);
    }
    char *table_names[4] = {"PML4", "PDPT", "PD", "PT"};
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virt_addr),
        GET_PDPT_IDX((uint64_t)virt_addr),
        GET_PD_IDX((uint64_t)virt_addr),
        GET_PT_IDX((uint64_t)virt_addr),
    };

    page_map_level_4 *pml4 = (page_map_level_4 *)page_table->page_dir;

    page_table_entry64 *table = pml4->e;
    if (!paging_disable_print)
    {
        log_info(MODULE, "trying to find pml4[%u]->pdpt[%u]->pd[%u]->pt[%u]", idx[0], idx[1], idx[2], idx[3]);
    }

    for (int level = 4; level >= (int)target_level; level--)
    {
        int current_level = 4 - level;
        char *current_table_name = table_names[current_level];
        int current_index = idx[current_level];
        if (level == (int)target_level)
        {
            if (!paging_disable_print)
            {
                log_info(MODULE, "found %s @ %p", current_table_name, table);
            }
            return (void *)table;
            // cast to page_map_level_4/page_directory64/etc. at call site
        }
        page_table_entry64 *entry = &table[current_index];
        if (!paging_disable_print)
        {
            log_debug(MODULE, "%s[%u] @ %p", current_table_name, current_index, entry);
            log_debug(MODULE, "%s[%u] = %03lx-%010lx-%03lx", current_table_name, current_index, (entry->raw >> 52) & 0xFFF, entry->addr, entry->raw & 0xFFF);
        }

        if (!entry->present)
        {
            if (!alloc)
            {
                return NULL;
            }

            if (!paging_disable_print)
            {
                log_debug(MODULE, "allocating new %s table entry in %s at index %u", table_names[4 - (level - 1)], current_table_name, current_index);
            }

            paddr_t new_phys;
            new_phys = pmm_alloc_frame();
            if (!new_phys)
            {
                return NULL;
            }

            vaddr_t new_virt = phys_to_virt_auto(new_phys);
            memset((void *)new_virt, 0, PAGE_SIZE);

            paging64_make_entry(entry, new_phys, level, flags);

            if (!paging_disable_print)
            {
                paddr_t entry_slot_phys = virt_to_phys_auto((vaddr_t)entry);
                log_debug(MODULE, "wrote entry@v%p (phys=%p) raw=0x%llx [table=%s idx=%u]", entry, entry_slot_phys, entry->raw, current_table_name, current_index);

                log_debug(MODULE, "%s[%u] = %03lx-%010lx-%03lx", current_table_name, current_index, (entry->raw >> 52) & 0xFFF, entry->addr, entry->raw & 0xFFF);
            }
        }
        else
        {
            if (entry->ps)
            {
                if (level == PAGING_LEVEL_PDPT)
                {
                    log_err(MODULE, "entry is a huge PDPT");
                    return NULL;
                }
                else
                {
                    log_err(MODULE, "entry is a huge PD");
                    return NULL;
                }
            }
        }

        paddr_t next_phys = (paddr_t)((uint64_t)entry->addr << 12);
        page_map_level_4 *next_table_entry = ((page_map_level_4 *)get_next(entry, level));
        if (!paging_disable_print)
        {
            log_debug(MODULE, "next level table at v%p/p%p", next_table_entry, next_phys, entry->addr);

            if (next_table_entry != NULL)
            {
                paddr_t reconverted = virt_to_phys_auto((vaddr_t)next_table_entry);
                if (reconverted != next_phys)
                {
                    log_err(MODULE, "[DIVERGENCE] entry->addr says phys=%p but get_next()->virt_to_phys gives %p",
                            next_phys, reconverted);
                }
            }
        }
        table = next_table_entry->e;
    }

    return NULL; // unreachable
}

// Returns a pointer to the entry at the requested level, or NULL on failure.
// If alloc=1, missing intermediate tables are allocated and zeroed.
// If alloc=0, returns NULL if any intermediate table is missing.
page_table_entry64 *paging64_get_entry(page_table_t *page_table, paddr_t phys_addr, vaddr_t virt_addr, paging_level target_level, bool alloc, mmu_flags_t flags)
{
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virt_addr),
        GET_PDPT_IDX((uint64_t)virt_addr),
        GET_PD_IDX((uint64_t)virt_addr),
        GET_PT_IDX((uint64_t)virt_addr),
    };

    page_map_level_4 *pml4 = (page_map_level_4 *)page_table->page_dir;
    page_table_entry64 *table = pml4->e;
    if (!paging_disable_print)
    {
        log_debug(MODULE, "table at %p", table);
    }

    for (int level = 4; level > (int)target_level; level--)
    {
        if (!paging_disable_print)
        {
            log_debug(MODULE, "table[%u] at %p", idx[4 - level], &table[idx[4 - level]]);
        }
        page_table_entry64 *entry = &table[idx[4 - level]];
        if (!paging_disable_print)
        {
            log_debug(MODULE, "at level %u(%u): entry raw = 0x%08llx", level, idx[4 - level], entry->raw);
            log_debug(MODULE, "addr = p%p, flags = 0x%x", entry->addr << 12, entry->raw & PAGE_FLAGS_MASK);
        }
        if (!entry->present)
        {
            if (!alloc)
            {
                return NULL;
            }

            if (!paging_disable_print)
            {
                log_debug(MODULE, "allocating new at level %u at %u", level, idx[4 - level]);
            }

            paddr_t new_phys;
            if (level != PAGING_LEVEL_PT)
            {
                new_phys = pmm_alloc_frame();
                if (!new_phys)
                {
                    return NULL;
                }
            }
            else
            {
                new_phys = phys_addr;
            }

            vaddr_t new_virt = phys_to_virt_auto(new_phys);

            memset((void *)new_virt, 0, PAGE_SIZE);

            if (entry->ps)
            {
                if (level == PAGING_LEVEL_PDPT)
                {
                    log_err(MODULE, "entry is a huge PDPT");
                    return NULL;
                }
                else
                {
                    log_err(MODULE, "entry is a huge PD");
                    return NULL;
                }
            }
            paging64_make_entry(entry, new_phys, level, flags);
        }

        paddr_t next_phys = (paddr_t)((uint64_t)entry->addr << 12);
        page_map_level_4 *next_table_entry = ((page_map_level_4 *)get_next(entry, level));
        if (!paging_disable_print)
        {
            log_debug(MODULE, "next level table at v%p/p%p", next_table_entry, next_phys);
        }
        table = next_table_entry->e;
    }

    page_table_entry64 *result = &table[idx[4 - target_level]];
    if (!paging_disable_print)
    {
        log_debug(MODULE, "result level %u: entry raw = 0x%08llx", target_level, result->raw);
        log_debug(MODULE, "addr = p%p, flags = 0x%x", result->addr, result->raw & PAGE_FLAGS_MASK);
    }
    return result;
}

page_map_level_4 *paging64_get_pml4(page_table_t *page, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pml4 table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_map_level_4 *)paging64_get_table(page, v, PAGING_LEVEL_PML4, alloc, flags);
}

page_dpt *paging64_get_pdpt(page_table_t *page, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pdpt table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_dpt *)paging64_get_table(page, v, PAGING_LEVEL_PDPT, alloc, flags);
}

page_directory64 *paging64_get_pd(page_table_t *page, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pd table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_directory64 *)paging64_get_table(page, v, PAGING_LEVEL_PD, alloc, flags);
}

page_table64 *paging64_get_pt(page_table_t *page, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pt table at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table64 *)paging64_get_table(page, v, PAGING_LEVEL_PT, alloc, flags);
}

page_table_entry64 *paging64_get_pml4_entry(page_table_t *page, paddr_t p, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pml4 entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(page, p, v, PAGING_LEVEL_PML4, alloc, flags);
}

page_table_entry64 *paging64_get_pdpt_entry(page_table_t *page, paddr_t p, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pdpt entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(page, p, v, PAGING_LEVEL_PDPT, alloc, flags);
}

page_table_entry64 *paging64_get_pd_entry(page_table_t *page, paddr_t p, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pd entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(page, p, v, PAGING_LEVEL_PD, alloc, flags);
}

page_table_entry64 *paging64_get_pt_entry(page_table_t *page, paddr_t p, vaddr_t v, int alloc, mmu_flags_t flags)
{
    if (!paging_disable_print)
    {
        log_info(MODULE, "finding and try allocate (%s) pt entry at virt %p", alloc BOOL_TO_STRING, v);
    }
    return (page_table_entry64 *)paging64_get_entry(page, p, v, PAGING_LEVEL_PT, alloc, flags);
}

int map(page_table_t *page_table, paddr_t start_phys, vaddr_t start_virt, size_t size, mmu_flags_t flags)
{
    if (page_table->page_dir == 0)
    {
        page_table->page_dir = (vaddr_t)phys_to_virt_auto(page_table->page_dir_phys);
        page_map_level_4 *pml4 = (void *)page_table->page_dir;
        log_debug(MODULE, "pml4 = %p", pml4);
        for (size_t i = 0; i < PT64_ENTRIES; i++)
        {
            pml4->e[i].raw = 0;
        }
    }
    size_t total_size = ALIGN_2_UP(size, PAGE_SIZE);
    log_debug(MODULE, "map %p..%p for %u bytes", start_phys, start_virt, total_size);
    log_debug(MODULE, "map %p..%p-%p..%p", start_phys, start_virt, start_phys + total_size, start_virt + total_size);
    vaddr_t virt = (vaddr_t)ALIGN_2_UP((uint64_t)start_virt, PAGE_SIZE);
    uint64_t resolved_flags = PAGE_PRESENT | PAGE_WRITABLE;

    int pml4_idx = GET_PML4_IDX(virt);
    int pdpt_idx = GET_PDPT_IDX(virt);
    int pd_idx = GET_PD_IDX(virt);
    int pt_idx = GET_PT_IDX(virt);
    log_debug(MODULE, "indices pml4=%u pdpt=%u pd=%u pt=%u", pml4_idx, pdpt_idx, pd_idx, pt_idx);

    int pt_count = total_size / PAGE_SIZE;
    int pd_count = pt_count / PT64_ENTRIES;
    page_directory64 *pd = paging64_get_pd(page_table, start_virt, 1, flags);
    if (!pd)
    {
        ERRNO_RETURN(EINVAL, "pd is null, aborting");
    }

    log_debug(MODULE, "pt_count=%u, pd_count=%u", pt_count, pd_count);
    int frame_index = 0;
    if (pd_count == 0)
    {
        paddr_t phys = pmm_alloc_frame();
        if (phys == 0)
        {
            ERRNO_RETURN(ENOMEM, "OMM with %u entries left", pd_count);
        }
        vaddr_t v = phys_to_virt_auto(phys);
        log_debug(MODULE, "4K path: pd[%u] PT phys=%p virt=%p", pd_idx + 0, phys, v);
        uint64_t *new_entry = (uint64_t *)v;
        for (size_t j = 0; j < 512; j++)
        {
            new_entry[j] = 0;
        }
        pd->e[pd_idx + 0].raw = 0;
        pd->e[pd_idx + 0].present = 1;
        pd->e[pd_idx + 0].writable = 1;
        pd->e[pd_idx + 0].addr = (uint64_t)phys >> 12;
        page_table64 *pt = (page_table64 *)new_entry;
        for (size_t k = 0; k < PT64_ENTRIES; k++)
        {
            uint64_t target_index = (uint64_t)frame_index + ((uint64_t)start_phys >> 12);
            log_debug(MODULE, "pml4[%d]->pdpt[%d]->pd[%u]->pt[%u] mapping p%p..v%p", pml4_idx, pdpt_idx, pd_idx + 0, k, (uint64_t)target_index << 12, virt + frame_index * PAGE_SIZE);
            pt->e[k].raw = 0;
            pt->e[k].present = 1;
            pt->e[k].writable = 1;
            pt->e[k].addr = target_index;
            frame_index++;
        }
    }
    for (int i = 0; i < pd_count; i++)
    {
        log_debug(MODULE, "pd = %08llx", pd->e[pd_idx + i].raw);
        if (!pd->e[pd_idx + i].present)
        {
            if (pt_count < PT64_ENTRIES && pd_count < 10)
            {
                paddr_t phys = pmm_alloc_frame();
                if (phys == 0)
                {
                    ERRNO_RETURN(ENOMEM, "OMM with %u entries left", pd_count);
                }
                vaddr_t v = phys_to_virt_auto(phys);
                log_debug(MODULE, "4K path: pd[%u] PT phys=%p virt=%p", pd_idx + i, phys, v);
                uint64_t *new_entry = (uint64_t *)v;
                for (size_t j = 0; j < 512; j++)
                {
                    new_entry[j] = 0;
                }
                pd->e[pd_idx + i].raw = 0;
                pd->e[pd_idx + i].present = 1;
                pd->e[pd_idx + i].writable = 1;
                pd->e[pd_idx + i].addr = (uint64_t)phys >> 12;
                page_table64 *pt = (page_table64 *)new_entry;
                for (size_t k = 0; k < PT64_ENTRIES; k++)
                {
                    uint64_t target_index = (uint64_t)frame_index + ((uint64_t)start_phys >> 12);
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
                paddr_t huge_phys = (paddr_t)((uint64_t)start_phys + i * 0x200000);
                if (huge_phys == 0)
                {
                    ERRNO_RETURN(ENOMEM, "OMM with %u entries left", pd_count);
                }
                log_debug(MODULE, "2MiB path: pd[%u] phys=%p virt=%p", pd_idx + i, huge_phys, virt + i * 0x200000);
                pd_huge_entry64 *huge_pd = (pd_huge_entry64 *)(void *)(&pd->e[pd_idx + i]);
                huge_pd->raw = 0;
                huge_pd->present = 1;
                huge_pd->writable = 1;
                huge_pd->ps = 1;
                huge_pd->addr = (uint64_t)huge_phys >> 21;
                log_debug(MODULE, "pml4[%d]->pdpt[%d]->pd[%u] mapping huge entry @ %p = raw=0x%llx addr_field=%llx", pml4_idx, pdpt_idx, pd_idx + i, huge_pd, huge_pd->raw, huge_pd->addr);
            }
        }
        else
        {
            log_debug(MODULE, "pd[%u] already present, skipping", pd_idx + i);
            log_debug(MODULE, "pd[%u] = %08llx", pd_idx + i, pd->e[pd_idx + i].raw);
        }
    }
    log_info(MODULE, "mapped virt=%08llx -> phys=%08llx (pml4i=%u pdpti=%u pdi=%u pti=%u flags=%x)",
             start_virt, start_phys,
             GET_PML4_IDX((uint64_t)start_virt), GET_PDPT_IDX((uint64_t)start_virt),
             GET_PD_IDX((uint64_t)start_virt), GET_PT_IDX((uint64_t)start_virt), resolved_flags);

    return 0;
}

size_t allocate_leaf(page_table_t *page_table, vaddr_t virt, paddr_t phys, mmu_flags_t flags)
{
    int pml4_idx = GET_PML4_IDX(virt);
    int pdpt_idx = GET_PDPT_IDX(virt);
    int pd_idx = GET_PD_IDX(virt);
    int pt_idx = GET_PT_IDX(virt);

    uint64_t leaf_resolved_flags = mm_flags_to_pte(flags);
    uint64_t resolved_flags = leaf_resolved_flags & 0x0FF;

    if (!paging_disable_print)
    {
        log_debug(MODULE, "resolved flags=%llx", resolved_flags);
    }

    if (!paging_disable_print)
    {
        log_info(MODULE, "map_page virt=%p phys=%p flags=%lx leaf_flags=%lx pml4[%u]->pdpt[%u]->pd[%u]->pt[%u] to %p", virt, phys, resolved_flags, leaf_resolved_flags, pml4_idx, pdpt_idx, pd_idx, pt_idx, page_table->page_dir);
    }

#define PRINT(table, entry)                                                                                                                       \
    if (!table || !entry)                                                                                                                         \
    {                                                                                                                                             \
        log_err(MODULE, "%s[%u] %p is null", #table, table##_idx, entry);                                                                         \
        return 1;                                                                                                                                 \
    }                                                                                                                                             \
    if (!paging_disable_print)                                                                                                                    \
    {                                                                                                                                             \
        log_debug(MODULE, "%s @ %p = 0x%llx { addr = %p, flags = 0x%lx }", #entry, entry, entry->raw, entry->addr, entry->raw & PAGE_FLAGS_MASK); \
        log_debug(MODULE, "%s = %03lx-%010lx-%03lx", #entry, (entry->raw >> 52) & 0xFFF, entry->addr, entry->raw & 0xFFF);                        \
    }                                                                                                                                             \
    if (false)                                                                                                                                    \
    {                                                                                                                                             \
        uint64_t raw = entry->raw;                                                                                                                \
        raw &= PAGE_ADDR_MASK;                                                                                                                    \
        raw |= resolved_flags & 0xFF;                                                                                                             \
        entry->raw = raw;                                                                                                                         \
    }

#define PRINT_INT(table, entry)                                                                                                                             \
    PRINT(table, entry)                                                                                                                                     \
    {                                                                                                                                                       \
        uint64_t raw = entry->raw & 0x0FF;                                                                                                                  \
        if (!paging_disable_print)                                                                                                                          \
        {                                                                                                                                                   \
            log_debug(MODULE, "%s @ %p = 0x%llx { addr = %p, flags = 0x%lx }", #entry, entry, entry->raw, entry->addr << 12, entry->raw & PAGE_FLAGS_MASK); \
            log_debug(MODULE, "{ addr = %p, flags = 0x%lx }", entry->addr << 12, raw);                                                                      \
        } /* entry->raw = raw */;                                                                                                                           \
        /* log_debug(MODULE, "{ addr = %p, flags = 0x%lx }", entry->addr << 12, entry->raw & PAGE_FLAGS_MASK); */                                           \
    }

#define PRINT_LP(table, entry) \
    if (flags.large == true)   \
    {                          \
        entry->ps = 1;  \
    }                          \
    if (!entry->ps)     \
    {                          \
        PRINT(table, entry)    \
    }

#define ALLOCATE_ENTRY(entry, new_table) \
    if (entry->raw == 0)                 \
    {                                    \
        paddr_t p = pmm_alloc_frame();   \
        entry->raw = resolved_flags;     \
        entry->addr = p >> 12;           \
        new_table = p;                   \
    }                                    \
    else                                 \
    {                                    \
        new_table = entry->addr << 12;   \
    }

    paddr_t pdpt_phys = 0;
    paddr_t pd_phys = 0;
    paddr_t pt_phys = 0;

    page_map_level_4 *pml4 = paging64_get_pml4(page_table, virt, 1, flags);
    page_table_entry64 *pml4e = &pml4->e[pml4_idx];
    ALLOCATE_ENTRY(pml4e, pdpt_phys);
    PRINT_INT(pml4, pml4e);

    page_dpt *pdpt = (page_dpt *)phys_to_virt_auto(pdpt_phys);
    page_table_entry64 *pdpte = &pdpt->e[pdpt_idx];
    ALLOCATE_ENTRY(pdpte, pd_phys);
    PRINT_INT(pdpt, pdpte);
    if (pdpte->ps)
    {
        KERNEL_PANIC(MODULE, "pdpt entry is huge and that was   not expected");
    }

    pd_phys = pdpte->addr << 12;
    page_directory64 *pd = (page_directory64 *)phys_to_virt_auto(pd_phys);
    page_table_entry64 *pde = &pd->e[pd_idx];
    ALLOCATE_ENTRY(pde, pt_phys);
    PRINT_LP(pd, pde);
    if (pde->ps || flags.large == 1)
    {
        log_warn(NO_MODULE, "============== MAPPING WITH A LARGE PAGE ==============");
        pd_huge_entry64 *h_entry = (pd_huge_entry64 *)pde;
        h_entry->raw = leaf_resolved_flags & PAGE_FLAGS_MASK;
        h_entry->addr = (uint64_t)phys >> 21;
        h_entry->ps = 1;
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pml4e->addr << 12, pml4e->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pdpte->addr << 12, pdpte->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pde->addr << 21, pde->raw & PAGE_FLAGS_MASK);
        goto done_map;
    }

    pt_phys = pde->addr << 12;
    page_table64 *pt = (page_table64 *)phys_to_virt_auto(pt_phys);
    page_table_entry64_leaf *pte = &pt->e[pt_idx];

    // pml4e->raw = resolved_flags & PAGE_FLAGS_MASK;
    // pml4e->addr = ((uint64_t)pdpte >> 12) & PAGE_ADDR_MASK;
    // 000a
    // 1001

    if (!pt || !pte)
    {
        log_err(MODULE, "pt is null");
        return 1;
    }

    if (pte->present)
    {
        log_err(MODULE, "Failed to allocate page table for virt 0x%llx", virt);
        log_err(MODULE, "Remapping already-mapped page at virt 0x%llx", virt);
        log_debug(MODULE, "pt @ %p = 0x%llx { addr = %p, flags = 0x%lx }", pte, pte->raw, pte->addr << 12, pte->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "{ addr = %p, flags = 0x%lx }", pte->addr << 12, pte->raw);
        log_err(MODULE, "pt[%u] is already present @ %p", pt_idx, pte);
        return 1;
    }

    if (!paging_disable_print)
    {
        log_debug(MODULE, "got entry at v%p from table %p", pte, pt);
    }

    paging64_make_entry((page_table_entry64 *)pte, phys, PAGING_LEVEL_PT, flags);
    log_debug(MODULE, "pte @ %p = 0x%llx { addr = %p, flags = 0x%lx }", pte, pte->raw, pte->addr, pte->raw & PAGE_FLAGS_MASK);

    if (!paging_disable_print)
    {
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pml4e->addr << 12, pml4e->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pdpte->addr << 12, pdpte->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pde->addr << 12, pde->raw & PAGE_FLAGS_MASK);

        log_debug(MODULE, "PT[%u] readback = 0x%llx", pt_idx, pte->raw);
    }

    if (!paging_disable_print)
    {
        log_debug(MODULE, "pte = %03lx-%010lx-%03lx", (pte->raw >> 52) & 0xFFF, pte->addr, pte->raw & 0xFFF);
    }
done_map:
    if (!paging_disable_print)
    {
        log_info(MODULE, "mapped virt=%p -> phys=%p (pml4i=%u pdpti=%u pdi=%u pti=%u flags=%llx)", virt, phys, pml4_idx, pdpt_idx, pd_idx, pt_idx, resolved_flags);
    }

    // Invalidate the TLB entry for this address
    mmu_arch_flush_page(virt);
    if (pdpte->ps)
    {
        return PAGE_SIZE_HUGE;
    }
    else if (pde->ps)
    {
        return PAGE_SIZE_LARGE;
    }
    return PAGE_SIZE;
}

INTERNAL INLINE int paging64_table_is_empty(page_table_entry64 *entries, int level)
{
    for (int i = 0; i < 512; i++)
    {
        if (level == PAGING_LEVEL_PT)
        {
            page_table_entry64_leaf *entries_leaf = (page_table_entry64_leaf *)entries;
            if (entries_leaf[i].destroy)
            {
                entries_leaf[i].raw = 0;
            }
        }
        if (entries[i].present)
        {
            return 0;
        }
    }
    return 1;
}

int paging_clean_up(page_table_t *page_table, vaddr_t virtAddr)
{
    vaddr_t virt = virtAddr;
    uint64_t pml4_index = GET_PML4_IDX(virt);
    uint64_t pdpt_index = GET_PDPT_IDX(virt);
    uint64_t pd_index = GET_PD_IDX(virt);
    uint64_t pt_index = GET_PT_IDX(virt);

    log_debug(MODULE, "paging64_clean_up: virt=0x%016llx [pml4=%llu pdpt=%llu pd=%llu pt=%llu]",
              virtAddr, pml4_index, pdpt_index, pd_index, pt_index);

    // Fetch all four tables (no alloc — if any is missing, nothing to unmap)
    page_map_level_4 *pml4 = paging64_get_pml4(page_table, virtAddr, 0, flags_none);
    page_dpt *pdpt = paging64_get_pdpt(page_table, virtAddr, 0, flags_none);
    page_directory64 *pd = paging64_get_pd(page_table, virtAddr, 0, flags_none);
    page_table64 *pt = paging64_get_pt(page_table, virtAddr, 0, flags_none);

    if (!pt)
    {
        ERRNO_RETURN(EPERM, "no PT for virt=0x%016llx, nothing to clean up", virtAddr);
    }
    
    page_table_entry64 *pml4_entry = &pml4->e[pml4_index];
    page_table_entry64 *pdpt_entry = &pdpt->e[pdpt_index];
    page_table_entry64 *pd_entry = &pd->e[pd_index];
    
    if (!pt->e[pt_index].present)
    {
        ERRNO_RETURN(EPERM, "no PT for virt=0x%016llx, nothing to clean up", virtAddr);
    }

    // Grab physical addresses before we zero the entries
    paddr_t pt_phys = (paddr_t)(pd_entry->addr << 12);
    paddr_t pd_phys = (paddr_t)(pdpt_entry->addr << 12);
    paddr_t pdpt_phys = (paddr_t)(pml4_entry->addr << 12);

    // Walk back up, freeing empty tables
    if (paging64_table_is_empty((page_table_entry64 *)pt->e, PAGING_LEVEL_PT))
    {
        log_debug(MODULE, "paging64_clean_up: PT empty, freeing PT phys=0x%016llx", pt_phys);
        pd_entry->raw = 0;
        pmm_free_frame(pt_phys);

        if (paging64_table_is_empty(pd->e, PAGING_LEVEL_PD))
        {
            log_debug(MODULE, "paging64_clean_up: PD empty, freeing PD phys=0x%016llx", pd_phys);
            pdpt_entry->raw = 0;
            pmm_free_frame(pd_phys);

            if (paging64_table_is_empty(pdpt->e, PAGING_LEVEL_PDPT))
            {
                log_debug(MODULE, "paging64_clean_up: PDPT empty, freeing PDPT phys=0x%016llx", pdpt_phys);
                pml4_entry->raw = 0;
                pmm_free_frame(pdpt_phys);
            }
        }
    }

    mmu_arch_flush_page(virtAddr);
    log_debug(MODULE, "paging64_clean_up: TLB flushed for virt=0x%016llx", virtAddr);
    return 0;
}

// Unmap a single virtual page.
paddr_t paging_unmap_page(page_table_t *page_table, vaddr_t virtAddr)
{
    ENTER_FUNC(MODULE, "%p, %p", page_table, virtAddr);
    vaddr_t virt = virtAddr;
    uint64_t pml4_index = GET_PML4_IDX(virt);
    uint64_t pdpt_index = GET_PDPT_IDX(virt);
    uint64_t pd_index = GET_PD_IDX(virt);
    uint64_t pt_index = GET_PT_IDX(virt);

    log_debug(MODULE, "paging_unmap_page: virt=0x%016llx [pml4=%llu pdpt=%llu pd=%llu pt=%llu]",
              virtAddr, pml4_index, pdpt_index, pd_index, pt_index);

    // Fetch all four tables (no alloc — if any is missing, nothing to unmap)
    page_table64 *pt = paging64_get_pt(page_table, virtAddr, 0, flags_none);

    if (!pt)
    {
        ERRNO_RETURN(EPERM, "no PT for virt=0x%016llx, nothing to unmap", virtAddr);
    }

    page_table_entry64_leaf *pt_entry = &pt->e[pt_index];

    if (!pt_entry->present)
    {
        ERRNO_RETURN(EPERM, "PTE not present for virt=0x%016llx, skipping", virtAddr);
    }

    log_info(MODULE, "paging_unmap_page: unmapping virt=0x%016llx", virtAddr);

    paddr_t result = (paddr_t)((pt_entry->addr << 12) | (virt & 0xFFF));

    pt_entry->destroy = 1;

    mmu_arch_flush_page(virtAddr);
    log_debug(MODULE, "paging_unmap_page: TLB flushed for virt=0x%016llx", virtAddr);
    return result;
}

// Walk the page directory and return the physical address mapped at virtAddr,
// or 0 if not mapped.
paddr_t paging_get_physical(page_table_t *page_table, vaddr_t virt)
{
#define PRINT_ENTRY(entry)                                                                                                                   \
    log_debug(MODULE, "%s @ %p = 0x%llx { addr = %p, flags = 0x%lx }", #entry, entry, entry->raw, entry->addr, entry->raw &PAGE_FLAGS_MASK); \
    log_debug(MODULE, "%s = %03lx-%010lx-%03lx", #entry, (entry->raw >> 52) & 0xFFF, entry->addr, entry->raw & 0xFFF);

    ENTER_FUNC("%p, %p", page_table, virt);
    page_map_level_4 *pml4 = (page_map_level_4 *)page_table->page_dir;

    int pml4_idx = GET_PML4_IDX(virt);
    int pdpt_idx = GET_PDPT_IDX(virt);
    int pd_idx = GET_PD_IDX(virt);
    int pt_idx = GET_PT_IDX(virt);

    if (!pml4 || pml4->e[pml4_idx].present == 0)
    {
        return 0;
    }
    page_table_entry64 *pml4e = &pml4->e[pml4_idx];

    // PRINT_ENTRY(pml4e);

    page_dpt *pdpt = paging64_get_pdpt(page_table, virt, false, flags_none);
    page_table_entry64 *pdpte = &pdpt->e[pdpt_idx];
    if (!pdpte || pdpte->present == 0)
    {
        return 0;
    }

    // PRINT_ENTRY(pdpte);

    // 1 GiB huge page
    if (pdpte->ps)
    {
        return (paddr_t)((pdpte->addr << 30) | (virt & 0x3FFFFFFF));
    }

    page_directory64 *pd = paging64_get_pd(page_table, virt, false, flags_none);
    page_table_entry64 *pde = &pd->e[pd_idx];
    if (!pde || pde->present == 0)
    {
        return 0;
    }

    // 2 MiB huge page
    if (pde->ps)
    {
        pd_huge_entry64 *huge_pde = (pd_huge_entry64 *)pde;
        // log_debug(MODULE, "huge pde @ %p = 0x%llx { addr = %p, flags = 0x%lx }", huge_pde, huge_pde->raw, huge_pde->addr, huge_pde->raw & PAGE_FLAGS_MASK);
        // log_debug(MODULE, "huge pde = %03lx-%010lx-%03lx", (huge_pde->raw >> 52) & 0xFFF, huge_pde->addr, huge_pde->raw & 0xFFF);
        return (paddr_t)((huge_pde->addr << 21) | (virt & 0x1FFFFF));
    }
    else
    {
        // PRINT_ENTRY(pde);
    }

    page_table64 *pt = paging64_get_pt(page_table, virt, false, flags_none);
    page_table_entry64_leaf *pte = &pt->e[pt_idx];
    if (!pte || pte->present == 0)
    {
        return 0;
    }

    // PRINT_ENTRY(pte);

    // 4 KiB page
    return (paddr_t)((pte->addr << 12) | (virt & 0xFFF));
}

paddr_t paging_print_info(page_table_t *page_dir, vaddr_t cr2)
{
    uint64_t PML4 = GET_PML4_IDX(cr2);
    uint64_t PDPT = GET_PDPT_IDX(cr2);
    uint64_t PD = GET_PD_IDX(cr2);
    uint64_t PT = GET_PT_IDX(cr2);
    log_debug_int(MODULE, "cr3->PML4[%u]->PDPT[%u]->PD[%u]->PT[%u]", PML4, PDPT, PD, PT);

    page_map_level_4 *pml4 = paging64_get_pml4(page_dir, cr2, 0, flags_none);
    page_dpt *pdpt = paging64_get_pdpt(page_dir, cr2, 0, flags_none);
    page_directory64 *pd = paging64_get_pd(page_dir, cr2, 0, flags_none);
    page_table64 *pt = paging64_get_pt(page_dir, cr2, 0, flags_none);
    page_table_entry64 *pml4_entry = &pml4->e[PML4];
    page_table_entry64 *pdpt_entry = &pdpt->e[PDPT];
    page_table_entry64 *pd_entry = &pd->e[PD];
    page_table_entry64_leaf *pt_entry = &pt->e[PT];
    log_debug_int(MODULE, "v%p/p%p PML4[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx", pml4, paging_get_physical(page_dir, (vaddr_t)pml4), PML4, pml4_entry->raw, pml4_entry->addr << 12, pml4_entry->raw & PAGE_FLAGS_MASK);
    if (!pml4_entry || pml4_entry->present == 0)
    {
        log_debug_int(MODULE, "The mapping is fucked\n");
        return 0;
    }
    log_debug_int(MODULE, "v%p/p%p PDPT[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx", pdpt, paging_get_physical(page_dir, (vaddr_t)pdpt), PDPT, pdpt_entry->raw, pdpt_entry->addr << 12, pdpt_entry->raw & PAGE_FLAGS_MASK);
    if (!pdpt_entry || pdpt_entry->present == 0)
    {
        log_debug_int(MODULE, "The mapping is fucked\n");
        return 0;
    }

    // 1 GiB huge page
    if (pdpt_entry->ps)
    {
        log_debug_int(MODULE, "%p PDPT[%u] is huge\n", pdpt, PDPT);
        return (paddr_t)((pdpt_entry->addr << 30) | (cr2 & 0x3FFFFFFF));
    }

    log_debug_int(MODULE, "v%p/p%p PD[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx", pd, paging_get_physical(page_dir, (vaddr_t)pd), PD, pd_entry->raw, pd_entry->addr << 12, pd_entry->raw & PAGE_FLAGS_MASK);
    if (!pd_entry || pd_entry->present == 0)
    {
        log_debug_int(MODULE, "The mapping is fucked\n");
        return 0;
    }

    // 2 MiB huge page
    if (pd_entry->ps)
    {
        log_debug_int(MODULE, "%p PD[%u] is huge\n", pd, PD);
        return (paddr_t)((pd_entry->addr << 21) | (cr2 & 0x1FFFFF));
    }

    log_debug_int(MODULE, "v%p/p%p PT[%u].raw = 0x%llx addr = phys0x%llx, flags = 0x%llx", pt, paging_get_physical(page_dir, (vaddr_t)pt), PT, pt_entry->raw, pt_entry->addr << 12, pt_entry->raw & PAGE_FLAGS_MASK);
    if (!pt_entry || pt_entry->present == 0)
    {
        log_debug_int(MODULE, "The mapping is fucked\n");
        return 0;
    }

    // 4 KiB page
    return (paddr_t)((pt_entry->addr << 12) | (cr2 & 0xFFF));
}

void paging_print_tree(page_table_t *page_dir)
{
    log_info(NO_MODULE, "walking page table %p and cr3 %p", page_dir, page_dir->page_dir_phys);
    page_map_level_4 *pml4 = (page_map_level_4 *)page_dir->page_dir;
    for (int pml4_idx = PT64_ENTRIES / 2; pml4_idx < PT64_ENTRIES; pml4_idx++)
    {
        page_table_entry64 *pml4e = &pml4->e[pml4_idx];
        if (pml4e->raw == 0)
        {
            int start_idx = pml4_idx;
            for (; pml4_idx < PT64_ENTRIES; pml4_idx++)
            {
                pml4e = &pml4->e[pml4_idx];
                if (pml4e->raw != 0)
                {
                    break;
                }
            }
            if (start_idx != pml4_idx - 1)
            {
                log_info(NO_MODULE, "pml4[%i..%i] = %p", start_idx, pml4_idx - 1, 0);
            }
            else
            {
                log_info(NO_MODULE, "pml4[%i] = %p", pml4_idx, pml4e->raw);
            }
        }
        if (pml4e->raw != 0)
        {
            log_info(NO_MODULE, "pml4[%i] = %p: phys addr = %p", pml4_idx, pml4e->raw, pml4e->addr << 12);
            paddr_t pdpt_phys = pml4e->addr << 12;
            page_dpt *pdpt = (page_dpt *)phys_to_virt_auto(pdpt_phys);
            for (size_t pdpt_idx = 0; pdpt_idx < PT64_ENTRIES; pdpt_idx++)
            {
                page_table_entry64 *pdpte = &pdpt->e[pdpt_idx];
                if (pdpte->raw == 0)
                {
                    int start_idx = pdpt_idx;
                    for (; pdpt_idx < PT64_ENTRIES; pdpt_idx++)
                    {
                        pdpte = &pdpt->e[pdpt_idx];
                        if (pdpte->raw != 0)
                        {
                            break;
                        }
                    }
                    if (start_idx != pdpt_idx - 1)
                    {
                        log_info(NO_MODULE, "\tpdpt[%i..%i] = %p", start_idx, pdpt_idx - 1, 0);
                    }
                    else
                    {
                        log_info(NO_MODULE, "\tpdpt[%i] = %p", pdpt_idx - 1, 0);
                    }
                }
                if (pdpte->raw != 0)
                {
                    log_info(NO_MODULE, "\tpdpt[%i] = %p: phys addr = %p", pdpt_idx, pdpte->raw, pdpte->addr << 12);
                    if (pdpte->ps == 1)
                    {
                        continue;
                    }
                    paddr_t pd_phys = pdpte->addr << 12;
                    page_directory64 *pd = (page_directory64 *)phys_to_virt_auto(pd_phys);
                    for (size_t pd_idx = 0; pd_idx < PT64_ENTRIES; pd_idx++)
                    {
                        page_table_entry64 *pde = &pd->e[pd_idx];
                        if (pde->raw == 0)
                        {
                            int start_idx = pd_idx;
                            for (; pd_idx < PT64_ENTRIES; pd_idx++)
                            {
                                pde = &pd->e[pd_idx];
                                if (pde->raw != 0)
                                {
                                    break;
                                }
                            }
                            if (start_idx != pd_idx - 1)
                            {
                                log_info(NO_MODULE, "\t\tpd[%i..%i] = %p", start_idx, pd_idx - 1, 0);
                            }
                            else
                            {
                                log_info(NO_MODULE, "\t\tpd[%i] = %p", pd_idx - 1, pde->raw);
                            }
                        }
                        if (pde->raw != 0)
                        {
                            if (pde->ps == 1)
                            {
                                pd_huge_entry64 *huge_pd = (pd_huge_entry64 *)pde;
                                log_info(NO_MODULE, "\t\tpd[%i] = %p: phys addr = %p", pd_idx, pde->raw, huge_pd->addr << 21);
                                continue;
                            }
                            else
                            {
                                log_info(NO_MODULE, "\t\tpd[%i] = %p: phys addr = %p", pd_idx, pde->raw, pde->addr << 12);
                            }
                            
                            paddr_t pt_phys = pde->addr << 12;
                            page_table64 *pt = (page_table64 *)phys_to_virt_auto(pt_phys);
                            for (size_t pt_idx = 0; pt_idx < PT64_ENTRIES; pt_idx++)
                            {
                                page_table_entry64_leaf *pte = &pt->e[pt_idx];
                                if (pte->raw == 0)
                                {
                                    int start_idx = pt_idx;
                                    for (; pt_idx < PT64_ENTRIES; pt_idx++)
                                    {
                                        pte = &pt->e[pt_idx];
                                        if (pte->raw != 0)
                                        {
                                            break;
                                        }
                                    }
                                    if (start_idx != pt_idx - 1)
                                    {
                                        log_info(NO_MODULE, "\t\t\tpt[%i..%i] = %p", start_idx, pt_idx - 1, 0);
                                    }
                                    else
                                    {
                                        log_info(NO_MODULE, "\t\t\tpt[%i] = %p", pt_idx - 1, pte->raw);
                                    }
                                }
                                if (pte->raw != 0)
                                {
                                    log_info(NO_MODULE, "\t\t\tpt[%i] = %p: phys addr = %p", pt_idx, pte->raw, pte->addr << 12);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

#endif
