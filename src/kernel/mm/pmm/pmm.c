/*
 * File: pmm.c
 * File Created: 01 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/pmm/pmm.h"
#include "mm/pmm/pmm_info.h"
#include "mm/pmm/pmm_register.h"
#include "mm/allocator/memory_allocator.h"
#include "mm/memdefs.h"

#include "assert/assert.h"
#include "memory.h"
#include "align.h"
#include "panic.h"
#include "debug/debug.h"
#include "asm/page_arch.h"
#include "asm/mmu_arch.h"

#define MODULE "pmm"

static paddr_t bump_base;
static paddr_t bump_current;
static paddr_t bump_end;
static bool pmm_ready = false;

pmm_info_t info;

extern char pmm_early_heap_size;

frame_allocator_t registry[CONFIG_MAX_FRAME_ALLOCATORS];

uint16_t *pmm_refcounts = NULL;
size_t pmm_frame_count = 0;

status_t pmm_early_init(boot_params_t *bp)
{
    pmm_ready = false;
    size_t early_size = (size_t)&pmm_early_heap_size;

    memory_entry_t *memory = bp->memory.entries;
    uint32_t entry_count = bp->memory.count;

    if (entry_count == 0)
    {
        KERNEL_PANIC(MODULE, "memory layout count is 0");
    }

    memory_entry_t *entry = NULL;
    size_t mem_size = 0;
    paddr_t kernel_phys_end = bp->kernel_phys_base + bp->kernel_size;
    info.kernel_virt_base = bp->kernel_virt_base;
    info.kernel_phys_base = bp->kernel_phys_base;

    trace_debug(MODULE, "pmm_init(%p)", bp);
    trace_debug(MODULE, "count = %u entries =%p", entry_count, memory);
    for (size_t i = 0; i < entry_count; i++)
    {
        entry = &memory[i];
        mem_size += entry->size;
    }
    info.mem_size = mem_size;
    for (size_t i = 0; i < entry_count; i++)
    {
        entry = &memory[i];

        trace_info(MODULE, "region entry start=%p size=%p type=%u", entry->addr, entry->size,
                  entry->type);

        if (entry->type != MEMORY_AVAILABLE)
        {
            continue;
        }
        if (entry->size < 0x2000000 + early_size)
        {
            continue;
        }
        if (entry->addr < 0x100000)
        {
            continue;
        }
        if (entry->size < early_size)
        {
            continue;
        }
        info.phys_start = (paddr_t)PAGE_ALIGN_UP((paddr_t)entry->addr);
        info.phys_end = (paddr_t)PAGE_ALIGN_UP((paddr_t)(entry->addr + entry->size));

        if (entry->addr < (paddr_t)kernel_phys_end)
        {
            info.phys_start = (paddr_t)PAGE_ALIGN_UP((paddr_t)kernel_phys_end);
            trace_debug(MODULE, "region start=%p end=%p", info.phys_start, info.phys_end);
        }
        
        info.memory = entry;
        
        bump_base = info.phys_start;
        bump_current = bump_base;
        bump_end = bump_base + early_size;
        info.phys_start = bump_end;
        info.phys_start = (paddr_t)ALIGN_2_UP(info.phys_start, 0x200000);
        break;
    }

    info.virt_start = info.phys_start + (info.kernel_virt_base - info.kernel_phys_base);
    trace_debug(MODULE, "info.virt_start=%p + (%p - %p)", info.phys_start, info.kernel_virt_base, info.kernel_phys_base);
    trace_debug(MODULE, "info.virt_start=%p", info.virt_start);

    trace_debug(MODULE, "bump_current = %p bump_end = %p", bump_current, bump_end);
    trace_debug(MODULE, "start        = %p end      = %p", info.phys_start, info.phys_end);

    int selection_index = 0;
    pmm_reg_get_allocator(&info, registry, &selection_index);
    info.allocator = &registry[selection_index];

    trace_debug(MODULE, "bump_current = %p bump_end = %p", bump_current, bump_end);
    trace_debug(MODULE, "start        = %p end      = %p", info.phys_start, info.phys_end);

    return KERRNO_SUCCESSES;
}

status_t pmm_init()
{
    pmm_reg_initialize(&info, registry);

    info.allocator->ops.print_stat_verbose(info.allocator);

    pmm_frame_count = info.mem_size / PAGE_SIZE;
    trace_debug(MODULE, "pmm_frame_count = %u", pmm_frame_count);
    pmm_refcounts = (uint16_t *)phys_to_virt_auto(pmm_alloc_frames_contiguous(PAGE_ALIGN_UP(pmm_frame_count * sizeof(uint16_t)) / PAGE_SIZE));
    memset(pmm_refcounts, 0, pmm_frame_count * sizeof(uint16_t));

    pmm_ready = true;

    allocator_init();

    return KERRNO_SUCCESSES;
}

INTERNAL static paddr_t bump_alloc()
{
    if (bump_current >= bump_end)
    {
        KERRNO_RETURN(KERRNO_POSIX_ENOMEM, "early heap exhausted");
    }
    paddr_t frame = bump_current;
    bump_current += PAGE_SIZE;
    return frame;
}

static inline size_t pmm_frame_idx(paddr_t phys)
{
    return phys >> 12;
}

paddr_t pmm_alloc_frame()
{
    paddr_t phys;
    if (!pmm_ready)
    {
        trace_debug(MODULE, "Allocating from bump %p/%p, %p", bump_current, bump_end, &pmm_ready);
        phys = bump_alloc();
    }
    else
    {
        int state = info.allocator->ops.alloc(info.allocator, 1, &phys);
        if (state != 0)
        {
            trace_debug(MODULE, "OOM");
            return 0;
        }
        if (phys)
        {
            pmm_refcounts[pmm_frame_idx(phys)] = 1;
        }
    }
    trace_debug(MODULE, "got addr=%p", phys);
    return phys;
}

int pmm_free_frame(paddr_t addr)
{
    paddr_t phys_addr = PAGE_ALIGN_DOWN(addr);
    return info.allocator->ops.free(info.allocator, phys_addr, 0);
}

paddr_t pmm_alloc_frames_contiguous(size_t times)
{
    paddr_t base = pmm_alloc_frame();
    for (size_t i = 0; i < (times - 1); i++)
    {
        pmm_alloc_frame();
    }
    return base;
}

status_t pmm_ref_frame(paddr_t phys)
{
    trace_debug(MODULE, "phys = %p", phys);
    size_t idx = pmm_frame_idx(phys);
    ASSERT(idx < pmm_frame_count, "idx = %u, pmm_frame_count = %u\n", idx, pmm_frame_count);
    pmm_refcounts[idx]++;
    return KERRNO_SUCCESSES;
}

status_t pmm_deref_frame(paddr_t phys)
{
    paddr_t phys_addr = PAGE_ALIGN_DOWN(phys);
    size_t idx = pmm_frame_idx(phys_addr);
    ASSERT(idx < pmm_frame_count, "");
    ASSERT(pmm_refcounts[idx] > 0, "underflow"); // underflow = kernel bug
    pmm_refcounts[idx]--;
    if (pmm_refcounts[idx] == 0)
    {
        pmm_free_frame(phys_addr); // give it back to the buddy allocator
    }
    return KERRNO_SUCCESSES;
}

uint16_t pmm_get_refcount(paddr_t phys)
{
    return pmm_refcounts[pmm_frame_idx(phys)];
}

pmm_info_t *pmm_get_info()
{
    return &info;
}

void pmm_print_info()
{
    info.allocator->ops.print_stat(info.allocator);
}

void pmm_print_info_verbose()
{
    info.allocator->ops.print_stat_verbose(info.allocator);
}

vaddr_t phys_to_virt_auto(paddr_t p)
{
    // log_debug(MODULE, "phys_to_virt_auto(%p) : is %p < %p then kernel else heap", p, p, info.phys_start);
    vaddr_t result = 0;
    if (p < info.phys_start)
    {
        result = (vaddr_t)(p + (info.kernel_virt_base - info.kernel_phys_base));
    }
    else
    {
        // log_debug(MODULE, "p(%p) - offset(%p) = %p", p, info.phys_start, p - info.phys_start);
        result = (vaddr_t)(p - info.phys_start + MEMORY_DIRECT_MAP_VIRT_BASE);
    }
    // log_debug(MODULE, "result = %p", result);
    return result;
}

paddr_t virt_to_phys_auto(vaddr_t p)
{
    // log_debug(MODULE, "virt_to_phys_auto(%p) : is %p >= %p then heap else kernel", p, p, info.kernel_virt_base);
    if (p >= info.kernel_virt_base)
    {
        // log_debug(MODULE, "KERNEL");
        return (paddr_t)(p - (info.kernel_virt_base - info.kernel_phys_base));
    }
    else
    {
        // log_debug(MODULE, "HEAP p(%p) - offset(%p) = %p", p, info.phys_start, p - info.phys_start);
        return (paddr_t)(p + info.phys_start - MEMORY_DIRECT_MAP_VIRT_BASE);
    }
}
