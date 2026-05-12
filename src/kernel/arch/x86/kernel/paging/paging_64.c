/*
 * File: paging_64.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#ifdef __x86_64__
#include "paging_64.h"
#include "libs/string.h"
#include "task/process.h"
#include "memory/memdefs.h"
#include "memory/pmm/pmm.h"

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_PWT (1ULL << 3)
#define PAGE_PCD (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE_PAGES (1ULL << 7)
#define PDE_GLOBAL (1ULL << 7)
#define PAGE_NO_EXEC (1ULL << 63)

#define PAGE_SIZE 4096

#define GET_PML4_IDX(va) ((((uint64_t)va) >> 39) & 0x1FF)
#define GET_PDPT_IDX(va) ((((uint64_t)va) >> 30) & 0x1FF)
#define GET_PD_IDX(va) ((((uint64_t)va) >> 21) & 0x1FF)
#define GET_PT_IDX(va) ((((uint64_t)va) >> 12) & 0x1FF)
#define GET_PAGE_OFF(va) (((uint64_t)va) & 0xFFF)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL
#define PAGE_FLAGS_MASK 0xFFF0000000000FFFULL

#define MODULE "PAGING_64"

typedef enum
{
    PAGING_LEVEL_PML4 = 4,
    PAGING_LEVEL_PDPT = 3,
    PAGING_LEVEL_PD = 2,
    PAGING_LEVEL_PT = 1,
} paging_level;

__attribute__((aligned(4096))) cr3_t *kernel_cr3;

paging_page_t kernel_page;

bool paging_print_out = true;

paging_flags mm_flags_to_pte(memory_flags_t flags)
{
    paging_flags pte = PAGE_PRESENT;

    if (flags.user)
        pte |= PAGE_USER;
    if (flags.write)
        pte |= PAGE_WRITABLE;
    if (!flags.exec)
        pte |= PAGE_NO_EXEC;
    if (flags.cached)
        pte |= PAGE_PCD | PAGE_PWT;
    if (flags.global)
        pte |= PDE_GLOBAL;

    return pte;
}

memory_flags_t pte_to_mm_flags(paging_flags pte)
{
    memory_flags_t f = {0};
    f.read = (pte & PAGE_PRESENT) != 0;
    f.write = (pte & PAGE_WRITABLE) != 0;
    f.exec = (pte & PAGE_NO_EXEC) == 0;
    f.user = (pte & PAGE_USER) != 0;
    f.cached = (pte & PAGE_PCD) == 0;
    f.global = (pte & PDE_GLOBAL) != 0;
    return f;
}

void map_old(page_map_level_4 *pml4_virt, page_dpt *pdpt_virt, page_dpt *pdpt_phys, virt_addr virt, phys_addr phys, size_t size)
{
    int pml4_index = GET_PML4_IDX(virt); // should be 511
    int pdpt_index = GET_PDPT_IDX(virt); // should be 510
    int pd_index = GET_PD_IDX(virt);     // should be 0
    log_debug(MODULE, "map: virt=0x%llx phys=0x%llx size=0x%zx", virt, phys, size);
    log_debug(MODULE, "map: pml4[%d] pdpt[%d] pd[%d]", pml4_index, pdpt_index, pd_index);

    if (pml4_virt->e[pml4_index].present == 0)
    {
        if (pdpt_virt == NULL)
        {
            pdpt_phys = paging_get_kernel_frame();
            pdpt_virt = paging_phys_to_virt((phys_addr)pdpt_phys);
            log_debug(MODULE, "map: allocated new pdpt phys=0x%llx virt=0x%llx", (uint64_t)pdpt_phys, (uint64_t)pdpt_virt);
        }
        pml4_virt->e[pml4_index].raw = 0;
        pml4_virt->e[pml4_index].present = 1;
        pml4_virt->e[pml4_index].writable = 1;
        pml4_virt->e[pml4_index].addr = (uint64_t)pdpt_phys >> 12;
        log_debug(MODULE, "map: pml4[%d] -> pdpt phys=0x%llx", pml4_index, (uint64_t)pdpt_phys);
    }
    else
    {
        log_debug(MODULE, "map: pml4[%d] already present, addr=0x%llx", pml4_index, (uint64_t)pml4_virt->e[pml4_index].addr << 12);
    }

    page_directory64 *pd_virt;
    page_directory64 *pd_phys;
    if (pdpt_virt->e[pdpt_index].present == 0)
    {
        pd_phys = (page_directory64 *)paging_get_kernel_frame();
        pd_virt = paging_phys_to_virt(pd_phys);
        log_debug(MODULE, "map: allocated new pd phys=0x%llx virt=0x%llx", (uint64_t)pd_phys, (uint64_t)pd_virt);
        for (size_t i = 0; i < PT64_ENTRIES; i++)
        {
            pd_virt->e[i].raw = 0;
        }
        pdpt_virt->e[pdpt_index].raw = 0;
        pdpt_virt->e[pdpt_index].present = 1;
        pdpt_virt->e[pdpt_index].writable = 1;
        pdpt_virt->e[pdpt_index].addr = (uint64_t)pd_phys >> 12;
        log_debug(MODULE, "map: pdpt[%d] -> pd phys=0x%llx", pdpt_index, (uint64_t)pd_phys);
    }
    else
    {

        phys_addr addr = (phys_addr)(uint64_t)(pdpt_virt->e[pdpt_index].addr << 12);
        pd_phys = (page_directory64 *)addr;
        pd_virt = paging_phys_to_virt(addr);
        log_debug(MODULE, "map: pdpt[%d] already present, reusing pd phys=0x%llx virt=0x%llx", pdpt_index, (uint64_t)pd_phys, (uint64_t)pd_virt);
    }

    pd_huge_entry64 *entry;
    size_t aligned_size = ALIGN_2_UP(size, 0x200000);
    uint32_t pd_huge_count = aligned_size / 0x200000;
    uint32_t starting_index = (uint64_t)phys / 0x200000;
    log_debug(MODULE, "map: huge pd count=%u (%p) starting_index=%u (%p) (pd[%d]..pd[%d])",
              pd_huge_count, aligned_size, starting_index, phys, pd_index, pd_index + pd_huge_count - 1);

    for (size_t i = 0; i < pd_huge_count; i++)
    {
        entry = (pd_huge_entry64 *)&pd_virt->e[pd_index + i];
        entry->raw = 0;
        entry->present = 1;
        entry->writable = 1;
        entry->ps = 1;
        entry->addr = i + starting_index;
        // log_debug(MODULE, "map: pd[%d] -> phys=0x%llx (frame %u)",
        //           pd_index + i, (uint64_t)(i + starting_index) << 21, i + starting_index);
    }

    log_debug(MODULE, "map: done, mapped 0x%llx..0x%llx -> 0x%llx..0x%llx",
              (uint64_t)virt, (uint64_t)virt + aligned_size - 1,
              (uint64_t)phys, (uint64_t)phys + aligned_size - 1);
}

virt_addr get_next(page_table_entry64 *entry, paging_level level)
{
    if (!entry->present)
    {
        return NULL;
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

    phys_addr next_addr = (phys_addr)next;
    /*     if (entry->kernel)
        {
            log_debug(MODULE, "get_next using kernel offset from %p", next_addr);
            return paging_phys_to_virt(next_addr);
        }
        log_debug(MODULE, "get_next using heap offset from %p", next_addr); */
    return phys_to_virt_auto(next_addr);
}

void map(page_map_level_4 *pml4_virt, virt_addr virtAddr, phys_addr physAddr, size_t size)
{
    size_t total_size = ALIGN_2_UP(size, 0x200000);
    log_debug(MODULE, "map %p..%p for %u bytes", physAddr, virtAddr, total_size);
    log_debug(MODULE, "map %p..%p-%p..%p", physAddr, virtAddr, physAddr + total_size, virtAddr + total_size);
    virt_addr virt = (virt_addr)ALIGN_2_UP((uint64_t)virtAddr, PAGE_SIZE);
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
    phys_addr phys = paging_get_kernel_frame();                                     \
    virt_addr v = phys_to_virt_auto(phys);                                          \
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

    virt_addr pdpt_virt = get_next(&pml4->e[pml4_idx], PAGING_LEVEL_PML4);
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
    virt_addr pd_virt = get_next(&pdpt->e[pdpt_idx], PAGING_LEVEL_PDPT);
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
                phys_addr phys = paging_get_kernel_frame();
                virt_addr v = phys_to_virt_auto(phys);
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
                    log_debug(MODULE, "at pd[%u]->pt[%u] mapping p%p..v%p", pd_idx + i, k, (uint64_t)target_index << 12, virt + frame_index * PAGE_SIZE);
                    pt->e[k].raw = 0;
                    pt->e[k].present = 1;
                    pt->e[k].writable = 1;
                    pt->e[k].addr = target_index;
                    frame_index++;
                }
            }
            else
            {
                phys_addr huge_phys = (phys_addr)((uint64_t)physAddr + i * 0x200000);
                log_debug(MODULE, "2MiB path: pd[%u] phys=%p virt=%p", pd_idx + i, huge_phys, virt + i * 0x200000);
                pd_huge_entry64 *huge_pd = (pd_huge_entry64 *)(void *)(&pd->e[pd_idx + i]);
                huge_pd->raw = 0;
                huge_pd->present = 1;
                huge_pd->writable = 1;
                huge_pd->ps = 1;
                huge_pd->addr = (uint64_t)huge_phys >> 21;
                log_debug(MODULE, "huge entry raw=0x%llx addr_field=%llx", huge_pd->raw, huge_pd->addr);
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

void paging_init64(boot_params *boot, size_t kernel_size, phys_addr pmm_start, phys_addr pmm_end)
{
    uint64_t kernel_start_virt = boot->kernel_address;

    page_map_level_4 *kernel_pml4 = (page_map_level_4 *)paging_get_kernel_frame();
    page_dpt *kernel_pdpt = (page_dpt *)paging_get_kernel_frame();

    log_debug(MODULE, "kernel_pml4 = %p", kernel_pml4);

    page_map_level_4 *kernel_pml4_virt = paging_phys_to_virt((phys_addr)kernel_pml4);
    page_dpt *kernel_pdpt_virt = paging_phys_to_virt((phys_addr)kernel_pdpt);

    uint64_t kernel_start_phys = (uint64_t)paging_virt_to_phys((virt_addr)kernel_start_virt);
    log_debug(MODULE, "kernel_start_virt = %p kernel_start_phys = %p", kernel_start_virt, kernel_start_phys);

    for (size_t i = 0; i < PT64_ENTRIES; i++)
    {
        kernel_pml4_virt->e[i].raw = 0;
        kernel_pdpt_virt->e[i].raw = 0;
    }

    map(kernel_pml4_virt, (virt_addr)kernel_start_virt, (phys_addr)kernel_start_phys, kernel_size);
    phys_addr start_phys_aligned = pmm_start; // pmm_start gotten from the pmm allocator (buddy)
    phys_addr end_phys_aligned = pmm_end;     // pmm_end gotten from the pmm allocator (buddy)
    size_t pmm_size = ALIGN_2_UP(end_phys_aligned - start_phys_aligned, 0x1000000);
    log_warn(MODULE, "pmm_start = %p pmm_end = %p", pmm_start, pmm_end);
    map(kernel_pml4_virt, (virt_addr)MEMORY_DIRECT_MAP_VIRT_BASE, start_phys_aligned, pmm_size);

    kernel_page.page_dir = kernel_pml4_virt;
    kernel_page.page_dir_phys = kernel_pml4;
    log_debug(MODULE, "setting cr3");
    __asm__ volatile(
        "mov cr3, %0" ::"r"((uint64_t)kernel_pml4) : "rax");
    log_debug(MODULE, "kernel_page @%p", &kernel_page);
    reload_pages();
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
page_table_entry64 *paging64_get_entry(paging_page_t paging_page_t, virt_addr virtAddr, paging_level target_level, bool alloc, bool user)
{
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virtAddr),
        GET_PDPT_IDX((uint64_t)virtAddr),
        GET_PD_IDX((uint64_t)virtAddr),
        GET_PT_IDX((uint64_t)virtAddr),
    };

    page_table_entry64 *table = ((page_map_level_4 *)paging_page_t.page_dir)->e;
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
                return NULL;

            if (paging_print_out)
            {
                log_debug(MODULE, "allocating new at level %u at %u", level, idx[4 - level]);
            }

            phys_addr new_phys = paging_alloc_frame();
            if (paging_print_out)
            {
                log_debug(MODULE, "got frame %p", new_phys);
            }
            if (!new_phys)
            {
                return NULL;
            }

            virt_addr new_virt = phys_to_virt_auto(new_phys);
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

        phys_addr next_phys = (phys_addr)((uint64_t)entry->addr << 12);
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
void *paging64_get_table(paging_page_t paging_page_t, virt_addr virtAddr, paging_level target_level, int alloc, bool user)
{
    uint64_t idx[4] = {
        GET_PML4_IDX((uint64_t)virtAddr),
        GET_PDPT_IDX((uint64_t)virtAddr),
        GET_PD_IDX((uint64_t)virtAddr),
        GET_PT_IDX((uint64_t)virtAddr),
    };

    page_table_entry64 *table = ((page_map_level_4 *)paging_page_t.page_dir)->e;
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
                log_debug(MODULE, "result level %u at %016p", target_level, table);
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
                return NULL;

            if (paging_print_out)
            {
                log_debug(MODULE, "allocating new at level %u at index %u", level, idx[4 - level]);
            }

            phys_addr new_phys = paging_alloc_frame();
            if (!new_phys)
            {
                return NULL;
            }

            virt_addr new_virt = phys_to_virt_auto(new_phys);

            memset((void *)new_virt, 0, PAGE_SIZE);

            if (paging_print_out)
            {
                log_debug(MODULE, "setting the fields on %p", entry);
                entry->raw = 0xFF00FF00;
                if (entry->raw != 0xFF00FF00)
                {
                    log_err(MODULE, "entry can't be written too");
                }
                log_debug(MODULE, "setting the fields on %p@%08X", entry, entry->raw);
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

        phys_addr next_phys = (phys_addr)((uint64_t)entry->addr << 12);
        page_map_level_4 *next_table_entry = ((page_map_level_4 *)get_next(entry, level));
        if (paging_print_out)
        {
            log_debug(MODULE, "next level table at v%p/p%p", next_table_entry, next_phys, entry->addr);
        }
        table = next_table_entry->e;
    }

    return NULL; // unreachable
}

static inline page_map_level_4 *paging64_get_pml4(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pml4 table at virt %p", alloc BOOT_TO_STRING, v);
    return (page_map_level_4 *)paging64_get_table(p, v, PAGING_LEVEL_PML4, alloc, user);
}
static inline page_dpt *paging64_get_pdpt(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pdpt table at virt %p", alloc BOOT_TO_STRING, v);
    return (page_dpt *)paging64_get_table(p, v, PAGING_LEVEL_PDPT, alloc, user);
}
static inline page_directory64 *paging64_get_pd(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pd table at virt %p", alloc BOOT_TO_STRING, v);
    return (page_directory64 *)paging64_get_table(p, v, PAGING_LEVEL_PD, alloc, user);
}
static inline page_table64 *paging64_get_pt(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pt table at virt %p", alloc BOOT_TO_STRING, v);
    return (page_table64 *)paging64_get_table(p, v, PAGING_LEVEL_PT, alloc, user);
}

static inline page_table_entry64 *paging64_get_pml4_entry(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pml4 entry at virt %p", alloc BOOT_TO_STRING, v);
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PML4, alloc, user);
}
static inline page_table_entry64 *paging64_get_pdpt_entry(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pdpt entry at virt %p", alloc BOOT_TO_STRING, v);
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PDPT, alloc, user);
}
static inline page_table_entry64 *paging64_get_pd_entry(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pd entry at virt %p", alloc BOOT_TO_STRING, v);
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PD, alloc, user);
}
static inline page_table_entry64 *paging64_get_pt_entry(paging_page_t p, virt_addr v, int alloc, bool user)
{
    if (paging_print_out)
        log_info(MODULE, "finding and try allocate (%s) pt entry at virt %p", alloc BOOT_TO_STRING, v);
    return (page_table_entry64 *)paging64_get_entry(p, v, PAGING_LEVEL_PT, alloc, user);
}

void paging_create_user_directory(void *_proc)
{
    process_t *proc = (process_t *)_proc;

    page_map_level_4 *user_pml4 = (page_map_level_4 *)paging_alloc_frame();
    page_dpt *user_pdpt = (page_dpt *)paging_alloc_frame();
    page_directory64 *user_pd = (page_directory64 *)paging_alloc_frame();

    page_map_level_4 *pml4_virt = phys_to_virt_auto((phys_addr)(uint64_t)user_pml4);
    page_dpt *pdpt_virt = phys_to_virt_auto((phys_addr)(uint64_t)user_pdpt);
    page_directory64 *user_pd_virt = phys_to_virt_auto((phys_addr)(uint64_t)user_pd);

    for (size_t i = 0; i < PT64_ENTRIES; i++)
    {
        pml4_virt->e[i].raw = 0;
        pdpt_virt->e[i].raw = 0;
        user_pd_virt->e[i].raw = 0;
    }

    page_map_level_4 *kernel_pml4_virt = (page_map_level_4 *)kernel_page.page_dir;

    for (int i = 256; i < PT64_ENTRIES; i++)
    {
        pml4_virt->e[i].raw = kernel_pml4_virt->e[i].raw;
    }
    log_debug(MODULE, "pml4_virt = %p", pml4_virt);
    log_debug(MODULE, "pdpt_virt = %p", pdpt_virt);
    log_debug(MODULE, "user_pml4 = %p", user_pml4);
    log_debug(MODULE, "user_pdpt = %p", user_pdpt);
    log_debug(MODULE, "user_pd = %p", user_pd);
    log_debug(MODULE, "proc = %p", proc);

    // Wire PML4[0] -> PDPT
    pml4_virt->e[0].raw = 0;
    pml4_virt->e[0].addr = ((uint64_t)user_pdpt) >> 12; // PFN
    pml4_virt->e[0].present = 1;
    pml4_virt->e[0].writable = 1;
    pml4_virt->e[0].user = 1; // <-- user-accessible

    // Wire PDPT[0] -> PD
    pdpt_virt->e[0].raw = 0;
    pdpt_virt->e[0].addr = ((uint64_t)user_pd) >> 12;
    pdpt_virt->e[0].present = 1;
    pdpt_virt->e[0].writable = 1;
    pdpt_virt->e[0].user = 1;

    proc->page_dir.page_dir = pml4_virt;
    proc->page_dir.page_dir_phys = user_pml4;
    log_debug(MODULE, "proc->page_dir_phys = %p", proc->page_dir.page_dir_phys);
    log_debug(MODULE, "proc->page_dir_virt = %p", proc->page_dir.page_dir);
    return;
}
void paging_free_user_directory(void *_proc)
{
    process_t *proc = (process_t *)_proc;

    paging_free_frame((phys_addr)(uint32_64)proc->page_dir.page_dir_phys);
}

void paging_copy_kernel_mappings(void *_proc)
{
    log_debug(MODULE, "proc = %p", _proc);
    process_t *proc = (process_t *)_proc;
    page_map_level_4 *pml4_virt = (page_map_level_4 *)proc->page_dir.page_dir;
    log_debug(MODULE, "user pml4_virt = %p", pml4_virt);
    page_map_level_4 *kernel_pml4_virt = (page_map_level_4 *)kernel_page.page_dir;
    log_debug(MODULE, "kernel pml4_virt = %p", kernel_pml4_virt);

    // higher half starts at PML4 index 256
    for (int i = 256; i < 512; i++)
    {
        pml4_virt->e[i].raw = kernel_pml4_virt->e[i].raw;
    }
}

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
phys_addr paging_get_physical(paging_page_t paging_page_t, virt_addr virt)
{
    page_map_level_4 *pml4 = (page_map_level_4 *)paging_page_t.page_dir;

    int pml4_idx = GET_PML4_IDX(virt);
    int pdpt_idx = GET_PDPT_IDX(virt);
    int pd_idx = GET_PD_IDX(virt);
    int pt_idx = GET_PT_IDX(virt);

    if (!pml4 || pml4->e[pml4_idx].present == 0)
    {
        return (phys_addr)0;
    }

    page_dpt *pdpt = (page_dpt *)phys_to_virt_auto((phys_addr)(uint64_t)(pml4->e[pml4_idx].addr << 12));
    if (!pdpt || pdpt->e[pdpt_idx].present == 0)
    {
        return (phys_addr)0;
    }

    // 1 GiB huge page
    if (pdpt->e[pdpt_idx].ps_or_pat)
    {
        return (phys_addr)((pdpt->e[pdpt_idx].addr << 30) | ((uint64_t)virt & 0x3FFFFFFF));
    }

    page_directory64 *pd = (page_directory64 *)phys_to_virt_auto((phys_addr)(uint64_t)(pdpt->e[pdpt_idx].addr << 12));
    if (!pd || pd->e[pd_idx].present == 0)
    {
        return (phys_addr)0;
    }

    // 2 MiB huge page
    if (pd->e[pd_idx].ps_or_pat)
        return (phys_addr)((pd->e[pd_idx].addr << 21) | ((uint64_t)virt & 0x1FFFFF));

    page_table64 *pt = (page_table64 *)phys_to_virt_auto((phys_addr)(uint64_t)(pd->e[pd_idx].addr << 12));
    if (!pt || pt->e[pt_idx].present == 0)
    {
        return (phys_addr)0;
    }

    // 4 KiB page
    return (phys_addr)((pt->e[pt_idx].addr << 12) | ((uint64_t)virt & 0xFFF));
}

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
virt_addr paging_get_virtual(paging_page_t paging_page_t, phys_addr physAddr)
{
    uint64_t phys = (uint64_t)physAddr & ~0xFFF;
    page_map_level_4 *pml4 = (page_map_level_4 *)paging_page_t.page_dir;

    for (uint16_t pml4i = 0; pml4i < PT64_ENTRIES; pml4i++)
    {
        page_table_entry64 *pml4e = &pml4->e[pml4i];
        // log_debug(MODULE, "pml4[%u] = %08x", pml4i, pml4e->raw);
        if (!pml4e->present)
            continue;

        page_dpt *pdpt = (page_dpt *)phys_to_virt_auto((phys_addr)(uint64_t)(pml4e->addr << 12));

        for (uint16_t pdpti = 0; pdpti < PT64_ENTRIES; pdpti++)
        {
            page_table_entry64 *pdpte = &pdpt->e[pdpti];
            // log_debug(MODULE, "pdpt[%u] = %08x", pdpti, pdpte->raw);
            if (!pdpte->present)
                continue;

            // 1GiB huge page (PS=1)
            if (pdpte->ps_or_pat)
            {
                uint64_t page_phys = (uint64_t)pdpte->addr << 30;
                if ((phys & ~0x3FFFFFFFULL) == page_phys)
                {
                    virt_addr va = (virt_addr)(((uint64_t)pml4i << 39) |
                                               ((uint64_t)pdpti << 30) |
                                               (phys & 0x3FFFFFFF));
                    return (virt_addr)va;
                }
                continue;
            }

            page_directory64 *pd = (page_directory64 *)phys_to_virt_auto((phys_addr)(uint64_t)(pdpte->addr << 12));

            for (uint16_t pdi = 0; pdi < PT64_ENTRIES; pdi++)
            {
                page_table_entry64 *pde = &pd->e[pdi];
                if (!pde->present)
                    continue;

                // 2MiB huge page (PS=1)
                if (pde->ps_or_pat)
                {
                    uint64_t page_phys = (uint64_t)pde->addr << 21;
                    if ((phys & ~0x1FFFFFULL) == page_phys)
                    {
                        virt_addr va = (virt_addr)(((uint64_t)pml4i << 39) |
                                                   ((uint64_t)pdpti << 30) |
                                                   ((uint64_t)pdi << 21) |
                                                   (phys & 0x1FFFFF));
                        return (virt_addr)va;
                    }
                    continue;
                }

                page_table64 *pt = (page_table64 *)phys_to_virt_auto((phys_addr)(uint64_t)(pde->addr << 12));

                for (uint16_t pti = 0; pti < PT64_ENTRIES; pti++)
                {
                    page_table_entry64 *pte = &pt->e[pti];
                    if (!pte->present)
                        continue;

                    if (((uint64_t)pte->addr << 12) == phys)
                    {
                        virt_addr va = (virt_addr)(((uint64_t)pml4i << 39) |
                                                   ((uint64_t)pdpti << 30) |
                                                   ((uint64_t)pdi << 21) |
                                                   ((uint64_t)pti << 12));
                        log_debug(MODULE, "Done got pml4i %u pdpti %u pdi %u pti %u from p0x%llx",
                                  pml4i, pdpti, pdi, pti, physAddr);
                        return (virt_addr)va;
                    }
                }
            }
        }
    }

    // log_warn("Paging", "could not get virtual from %p", physAddr);
    return NULL;
}

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(paging_page_t paging_page_t, virt_addr virtAddr, phys_addr physAddr, memory_flags_t flags)
{
    virt_addr virt = (virt_addr)ALIGN_2_UP((uint64_t)virtAddr, PAGE_SIZE);

    int pml4_idx = GET_PML4_IDX(virt);
    int pdpt_idx = GET_PDPT_IDX(virt);
    int pd_idx = GET_PD_IDX(virt);
    int pt_idx = GET_PT_IDX(virt);

    uint64_t resolved_flags = PAGE_PRESENT | PAGE_WRITABLE;
    uint64_t resolved_flags_pt = PAGE_PRESENT | PAGE_WRITABLE;
    uint8_t raw_flags = *((uint8_t *)(void *)&flags);
    if (raw_flags != -1)
    {
        resolved_flags_pt = mm_flags_to_pte(flags);
        resolved_flags = mm_flags_to_pte(flags);
        resolved_flags &= ~PAGE_NO_EXEC;
        resolved_flags &= ~PDE_GLOBAL;
    }

    if (paging_print_out)
    {
        log_debug(MODULE, "resolved flags=%llx", resolved_flags);
    }

    if (paging_print_out)
    {
        log_info(MODULE, "map_page virt=%p phys=%p flags=%llx pml4i=%u pdpt=%u pdi=%u pti=%u to %p", virtAddr, physAddr, resolved_flags,
                 pml4_idx, pdpt_idx, pd_idx, pt_idx, paging_page_t.page_dir);
    }

    page_map_level_4 *pml4 = (page_map_level_4 *)paging_page_t.page_dir;

    if (!pml4)
    {
        return;
    }

    bool user_flag = ((resolved_flags & PAGE_USER) == PAGE_USER);
#define ALLOCATE_NEW_TABLE(entry)                                 \
    phys_addr phys = paging_alloc_frame();                        \
    virt_addr v = phys_to_virt_auto(phys);                        \
    if ((uint64_t)v == ALIGN_2_DOWN((uint64_t)&entry, PAGE_SIZE)) \
    {                                                             \
        log_err(MODULE, "something happened");                    \
        phys = paging_alloc_frame();                              \
        v = phys_to_virt_auto(phys);                              \
    }                                                             \
    uint64_t *new_entry = (uint64_t *)v;                          \
    for (size_t i = 0; i < PT64_ENTRIES; i++)                     \
    {                                                             \
        new_entry[i] = 0;                                         \
    }                                                             \
    entry.raw = 0;                                                \
    entry.raw = resolved_flags;                                   \
    entry.user = user_flag;                                       \
    entry.addr = (uint64_t)phys >> 12;

    if (!pml4->e[pml4_idx].present)
    {
        ALLOCATE_NEW_TABLE(pml4->e[pml4_idx]);
        log_debug(MODULE, "pml4e = 0x%llx @ %p", pml4->e[pml4_idx].raw, &pml4->e[pml4_idx]);
    }
    else
    {
        if (paging_print_out)
        {
            log_debug(MODULE, "pml4e = 0x%llx @ %p", pml4->e[pml4_idx].raw, &pml4->e[pml4_idx]);
            log_debug(MODULE, "addr = p%p, flags = 0x%llx", pml4->e[pml4_idx].addr << 12, pml4->e[pml4_idx].raw & PAGE_FLAGS_MASK);
        }
    }

    virt_addr pdpt_virt = get_next(&pml4->e[pml4_idx], PAGING_LEVEL_PML4);
    page_dpt *pdpt = (page_dpt *)pdpt_virt;
    if (!pdpt)
    {
        return;
    }

    if (!pdpt->e[pdpt_idx].present)
    {
        ALLOCATE_NEW_TABLE(pdpt->e[pdpt_idx]);
        log_debug(MODULE, "pdpte = 0x%llx @ %p", pdpt->e[pdpt_idx].raw, &pdpt->e[pdpt_idx]);
    }
    else
    {
        if (paging_print_out)
        {
            log_debug(MODULE, "pdpte = 0x%llx @ %p", pdpt->e[pdpt_idx].raw, &pdpt->e[pdpt_idx]);
            log_debug(MODULE, "addr = p%p, flags = 0x%llx", pdpt->e[pdpt_idx].addr << 12, pdpt->e[pdpt_idx].raw & PAGE_FLAGS_MASK);
        }
        // 1 GiB huge page
        if (pdpt->e[pdpt_idx].ps_or_pat)
        {
            return;
        }
    }

    virt_addr pd_virt = get_next(&pdpt->e[pdpt_idx], PAGING_LEVEL_PDPT);
    page_directory64 *pd = (page_directory64 *)pd_virt;
    if (!pd)
    {
        return;
    }

    if (!pd->e[pd_idx].present)
    {
        ALLOCATE_NEW_TABLE(pd->e[pd_idx]);
        log_debug(MODULE, "pde = 0x%llx @ %p", pd->e[pd_idx].raw, &pd->e[pd_idx]);
    }
    else
    {
        if (paging_print_out)
        {
            log_debug(MODULE, "pde = 0x%llx @ %p", pd->e[pd_idx].raw, &pd->e[pd_idx]);
            log_debug(MODULE, "addr = p%p, flags = 0x%llx", pd->e[pd_idx].addr << 12, pd->e[pd_idx].raw & PAGE_FLAGS_MASK);
        }
        // 2 MiB huge page
        if (pd->e[pd_idx].ps_or_pat)
        {
            log_err(MODULE, "pd is huge");
            return;
        }
    }

    phys_addr pt_phys = (phys_addr)(uint64_t)(pd->e[pd_idx].addr << 12);
    virt_addr pt_virt = phys_to_virt_auto(pt_phys);
    page_table64 *pt = (page_table64 *)pt_virt;
    if (!pt)
    {
        log_err(MODULE, "pt is null");
        return;
    }

    if (pt->e[pt_idx].present)
    {
        log_err(MODULE, "Failed to allocate page table for virt 0x%llx", (uint64_t)virtAddr);
        return;
    }

    page_table_entry64 *pt_entry = &pt->e[pt_idx];
    if (paging_print_out)
    {
        log_debug(MODULE, "got entry at v%p from table %p", pt_entry, pt);
    }

    if (pt_entry->present)
    {
        log_err(MODULE, "Remapping already-mapped page at virt 0x%llx", (uint64_t)virtAddr);
    }

    pt_entry->raw = resolved_flags_pt & PAGE_FLAGS_MASK;
    pt_entry->addr = (uint64_t)physAddr >> 12;

    if (paging_print_out)
    {
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pml4->e[pml4_idx].addr << 12, pml4->e[pml4_idx].raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pdpt->e[pdpt_idx].addr << 12, pdpt->e[pdpt_idx].raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "addr = p%p, flags = 0x%llx", pd->e[pd_idx].addr << 12, pd->e[pd_idx].raw & PAGE_FLAGS_MASK);

        log_debug(MODULE, "PT[%u] readback = 0x%llx", pt_idx, pt->e[pt_idx].raw);
    }

    if (paging_print_out)
    {
        log_debug(MODULE, "pte = 0x%08x/0b%064b", pt_entry->raw, pt_entry->raw);
        log_info(MODULE, "mapped virt=%p -> phys=%p (pml4i=%u pdpti=%u pdi=%u pti=%u flags=%llx)", virtAddr, physAddr, GET_PML4_IDX((uint64_t)virtAddr), GET_PDPT_IDX((uint64_t)virtAddr), GET_PD_IDX((uint64_t)virtAddr), GET_PT_IDX((uint64_t)virtAddr), resolved_flags);
    }

    // Invalidate the TLB entry for this address
    paging_native_flush_tlb_single(virtAddr);
#undef ALLOCATE_NEW_TABLE
}

// Unmap a single virtual page.
// Unmap a single virtual page.
void paging_unmap_page(paging_page_t paging_page_t, virt_addr virtAddr)
{
    virt_addr virt = virtAddr;
    uint64_t pml4_index = GET_PML4_IDX((uint64_t)virt);
    uint64_t pdpt_index = GET_PDPT_IDX((uint64_t)virt);
    uint64_t pd_index = GET_PD_IDX((uint64_t)virt);
    uint64_t pt_index = GET_PT_IDX((uint64_t)virt);

    log_debug(MODULE, "paging_unmap_page: virt=0x%016llx [pml4=%llu pdpt=%llu pd=%llu pt=%llu]",
              (uint64_t)virtAddr, pml4_index, pdpt_index, pd_index, pt_index);

    // Fetch all four tables (no alloc — if any is missing, nothing to unmap)
    page_map_level_4 *pml4 = paging64_get_pml4(paging_page_t, virtAddr, 0, false);
    page_dpt *pdpt = paging64_get_pdpt(paging_page_t, virtAddr, 0, false);
    page_directory64 *pd = paging64_get_pd(paging_page_t, virtAddr, 0, false);
    page_table64 *pt = paging64_get_pt(paging_page_t, virtAddr, 0, false);

    if (!pt)
    {
        log_err(MODULE, "paging_unmap_page: no PT for virt=0x%016llx, nothing to unmap",
                (uint64_t)virtAddr);
        return;
    }

    page_table_entry64 *pml4_entry = &pml4->e[pml4_index];
    page_table_entry64 *pdpt_entry = &pdpt->e[pdpt_index];
    page_table_entry64 *pd_entry = &pd->e[pd_index];
    page_table_entry64 *pt_entry = &pt->e[pt_index];

    if (!pt_entry->present)
    {
        log_err(MODULE, "paging_unmap_page: PTE not present for virt=0x%016llx, skipping",
                  (uint64_t)virtAddr);
        return;
    }

    // Grab physical addresses before we zero the entries
    phys_addr pt_phys = (phys_addr)(uint64_t)(pd_entry->addr << 12);
    phys_addr pd_phys = (phys_addr)(uint64_t)(pdpt_entry->addr << 12);
    phys_addr pdpt_phys = (phys_addr)(uint64_t)(pml4_entry->addr << 12);

    log_info(MODULE, "paging_unmap_page: unmapping virt=0x%016llx", (uint64_t)virtAddr);

    pt_entry->raw = 0;

    // Walk back up, freeing empty tables
    if (paging64_table_is_empty(pt->e))
    {
        log_debug(MODULE, "paging_unmap_page: PT empty, freeing PT phys=0x%016llx", (uint64_t)pt_phys);
        pd_entry->raw = 0;
        paging_free_frame(pt_phys);

        if (paging64_table_is_empty(pd->e))
        {
            log_debug(MODULE, "paging_unmap_page: PD empty, freeing PD phys=0x%016llx", (uint64_t)pd_phys);
            pdpt_entry->raw = 0;
            paging_free_frame(pd_phys);

            if (paging64_table_is_empty(pdpt->e))
            {
                log_debug(MODULE, "paging_unmap_page: PDPT empty, freeing PDPT phys=0x%016llx", (uint64_t)pdpt_phys);
                pml4_entry->raw = 0;
                paging_free_frame(pdpt_phys);
                // Never free pml4 itself — that's the root, caller owns it
            }
        }
    }

    paging_native_flush_tlb_single(virtAddr);
    log_debug(MODULE, "paging_unmap_page: TLB flushed for virt=0x%016llx", (uint64_t)virtAddr);
}

static spinlock_t page_copy_lock = {0};

void page_copy(phys_addr src, phys_addr dst)
{
    spinlock_acquire(&page_copy_lock);

    // map src and dst into scratch windows
    paging_map_page(kernel_page, MEMORY_PHYS_COPY_SRC, src, kernel_data_flags);
    paging_map_page(kernel_page, MEMORY_PHYS_COPY_DST, dst, kernel_data_flags);

    // flush TLB for both
    paging_native_flush_tlb_single(MEMORY_PHYS_COPY_SRC);
    paging_native_flush_tlb_single(MEMORY_PHYS_COPY_DST);

    memcpy((void *)MEMORY_PHYS_COPY_DST, (void *)MEMORY_PHYS_COPY_SRC, PAGE_SIZE);

    // unmap scratch windows
    paging_unmap_page(kernel_page, MEMORY_PHYS_COPY_SRC);
    paging_unmap_page(kernel_page, MEMORY_PHYS_COPY_DST);

    spinlock_release(&page_copy_lock);
}

void paging_print_info(paging_page_t page_dir, virt_addr cr2)
{
    uint64_t PML4 = GET_PML4_IDX((uint64_t)cr2);
    uint64_t PDPT = GET_PDPT_IDX((uint64_t)cr2);
    uint64_t PD = GET_PD_IDX((uint64_t)cr2);
    uint64_t PT = GET_PT_IDX((uint64_t)cr2);
    fprintf(VFS_FD_DEBUG, "cr3->PML4[%u]->PDPT[%u]->PD[%u]->PT[%u]\n", PML4, PDPT, PD, PT);

    cr3_t cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    page_map_level_4 *pml4 = paging64_get_pml4(page_dir, cr2, 0, 0);
    page_dpt *pdpt = paging64_get_pdpt(page_dir, cr2, 0, 0);
    page_directory64 *pd = paging64_get_pd(page_dir, cr2, 0, 0);
    page_table64 *pt = paging64_get_pt(page_dir, cr2, 0, 0);
    page_table_entry64 *pml4_entry = &pml4->e[PML4];
    page_table_entry64 *pdpt_entry = &pdpt->e[PDPT];
    page_table_entry64 *pd_entry = &pd->e[PD];
    page_table_entry64 *pt_entry = &pt->e[PT];
    fprintf(VFS_FD_DEBUG, "%016p PML4[%u].raw = 0x%llx addr = phys0x%08x, flags = 0x%llx\n", pml4, PML4, pml4_entry->raw, pml4_entry->addr << 12, pml4_entry->raw & PAGE_FLAGS_MASK);
    if (pml4_entry->raw == 0)
    {
        fprintf(VFS_FD_DEBUG, "The mapping is fucked\n");
        return;
    }
    fprintf(VFS_FD_DEBUG, "%016p PDPT[%u].raw = 0x%llx addr = phys0x%08x, flags = 0x%llx\n", pdpt, PDPT, pdpt_entry->raw, pdpt_entry->addr << 12, pdpt_entry->raw & PAGE_FLAGS_MASK);
    if (pdpt_entry->raw == 0)
    {
        fprintf(VFS_FD_DEBUG, "The mapping is fucked\n");
        return;
    }
    fprintf(VFS_FD_DEBUG, "%016p PD[%u].raw = 0x%llx addr = phys0x%08x, flags = 0x%llx\n", pd, PD, pd_entry->raw, pd_entry->addr << 12, pd_entry->raw & PAGE_FLAGS_MASK);
    if (pd_entry->raw == 0)
    {
        fprintf(VFS_FD_DEBUG, "The mapping is fucked\n");
        return;
    }
    fprintf(VFS_FD_DEBUG, "%016p PT[%u].raw = 0x%llx addr = phys0x%08x, flags = 0x%llx\n", pt, PT, pt_entry->raw, pt_entry->addr << 12, pt_entry->raw & PAGE_FLAGS_MASK);
}
#endif