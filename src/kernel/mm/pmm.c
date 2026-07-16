/*
 * File: pmm.c
 * File Created: 16 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pmm.h"

#include "task/assert/assert.h"

#include "buddy/buddy.h"

#include "debug/debug.h"
#include "kernel/mmu.h"
#include "kernel/memory.h"

#include "mm/memdefs.h"

#include <util/binary.h>

#define MODULE "PMM"

/* 5 MiB*/
#define EARLY_HEAP_SIZE 0x500000

static paddr_t bump_base;
static paddr_t bump_current;
static paddr_t bump_end;
static bool pmm_ready = false;

size_t mem_size;

static uint16_t *pmm_refcounts = NULL;
static size_t pmm_frame_count = 0;

vaddr_t phys_to_virt_auto(paddr_t p)
{
    // log_debug("PMM", "phys_to_virt_auto(%p) : is %p < %p then kernel else heap", p, p, start);
    if (p < start)
    {
        return (vaddr_t)(p + (KERNEL_VIRT_BASE - KERNEL_PHYS_BASE));
    }
    else
    {
        return (vaddr_t)(p - start + MEMORY_DIRECT_MAP_VIRT_BASE);
    }
}

paddr_t virt_to_phys_auto(vaddr_t p)
{
    // log_debug("PMM", "phys_to_virt_auto(%p) : is %p < %p then kernel else heap", p, p, start);
    if (p < start)
    {
        return (paddr_t)(p - (KERNEL_VIRT_BASE - KERNEL_PHYS_BASE));
    }
    else
    {
        return (paddr_t)(p - start - MEMORY_DIRECT_MAP_VIRT_BASE);
    }
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

paddr_t pmm_init(boot_params_t *params)
{
    bp_memory_t *memory = &params->memory;
    memory_entry_t *entry = NULL;

    log_debug(MODULE, "pmm_init(%p)", params);
    log_debug(MODULE, "count = %u entries =%p", memory->count, memory->entries);
    for (size_t i = 0; i < memory->count; i++)
    {
        entry = &memory->entries[i];
        mem_size += entry->size;
    }
    for (size_t i = 0; i < memory->count; i++)
    {
        entry = &memory->entries[i];

        log_debug(MODULE, "region entry start=%p size=%p", entry->addr, entry->size);

        if (entry->type != MEMORY_AVAILABLE)
        {
            continue;
        }
        if (entry->addr < 0x100000)
        {
            continue;
        }
        if (entry->size < EARLY_HEAP_SIZE)
        {
            continue;
        }
        start = (paddr_t)PAGE_ALIGN_UP((paddr_t)entry->addr);
        end = (paddr_t)PAGE_ALIGN_UP((paddr_t)(entry->addr + entry->size));

        if (entry->addr < (paddr_t)KERNEL_PHYS_END)
        {
            start = (paddr_t)PAGE_ALIGN_UP((paddr_t)KERNEL_PHYS_END);
            log_debug(MODULE, "region start=%p end=%p", start, end);
        }

        bump_base = PAGE_ALIGN_UP(start);
        bump_current = bump_base;
        bump_end = bump_base + EARLY_HEAP_SIZE;
        start = bump_end;
        break;
    }
    log_debug(MODULE, "bump_current = %p bump_end = %p", bump_current, bump_end);
    log_debug(MODULE, "start        = %p end      = %p", start, end);

    buddy_init(entry);

    log_debug(MODULE, "bump_current = %p bump_end = %p", bump_current, bump_end);
    log_debug(MODULE, "start        = %p end      = %p", start, end);

    pmm_frame_count = mem_size / PAGE_SIZE;
    // allocate the array from your PMM or a reserved region at boot
    log_debug(MODULE, "pmm_frame_count = %u", pmm_frame_count);
    pmm_refcounts = (uint16_t *)phys_to_virt_auto(pmm_alloc_frames_contiguous(
        PAGE_ALIGN_UP(pmm_frame_count * sizeof(uint16_t)) / PAGE_SIZE));
    memset(pmm_refcounts, 0, pmm_frame_count * sizeof(uint16_t));

    return bump_end;
}

static paddr_t bump_alloc()
{
    if (bump_current >= bump_end)
    {
        log_err(MODULE, "bump_current = %p bump_end = %p", bump_current, bump_end);
        KernelPanic(MODULE, "early heap exhausted");
    }
    paddr_t frame = bump_current;
    bump_current += PAGE_SIZE;
    return frame;
}

void pmm_map()
{
    buddy_map();
    pmm_ready = true;
}

static inline size_t pmm_frame_idx(paddr_t phys)
{
    return phys >> 12;
}

void pmm_ref_frame(paddr_t phys)
{
    log_debug(MODULE, "phys = %p", phys);
    size_t idx = pmm_frame_idx(phys);
    ASSERT(idx < pmm_frame_count, "idx = %u, pmm_frame_count = %u\n", idx, pmm_frame_count);
    pmm_refcounts[idx]++;
}

void pmm_deref_frame(paddr_t phys)
{
    paddr_t phys_addr = PAGE_ALIGN_DOWN(phys);
    ENTER_FUNC(MODULE, "%p", phys_addr);
    size_t idx = pmm_frame_idx(phys_addr);
    ASSERT(idx < pmm_frame_count, "");
    ASSERT(pmm_refcounts[idx] > 0, "underflow"); // underflow = kernel bug
    pmm_refcounts[idx]--;
    if (pmm_refcounts[idx] == 0)
    {
        pmm_free_frame(phys_addr); // give it back to the buddy allocator
    }
}

uint16_t pmm_get_refcount(paddr_t phys)
{
    return pmm_refcounts[pmm_frame_idx(phys)];
}

void pmm_print_info()
{
    buddy_print_info();
}

void pmm_print_info_verbose()
{
    buddy_print_info_verbose();
}

paddr_t pmm_alloc_region(paddr_t start_phys, paddr_t end_phys)
{
    paddr_t addr = (paddr_t)(uint32_64)ALIGN_DOWN((uint32_64)start_phys, PAGE_SIZE);
    paddr_t top = (paddr_t)(uint32_64)ALIGN_UP((uint32_64)end_phys, PAGE_SIZE);
    while (addr < top)
    {
        buddy_alloc_at(addr);
        addr += PAGE_SIZE;
    }
    return addr;
}

paddr_t pmm_alloc_at(paddr_t addr)
{
    paddr_t phys = buddy_alloc_at(addr);
    if (phys)
    {
        pmm_refcounts[pmm_frame_idx(phys)] = 1;
    }
    // not in any free list — already allocated, nothing to do
    return phys;
}

paddr_t pmm_alloc_frame()
{
    if (!pmm_ready)
    {
        return bump_alloc();
    }
    paddr_t phys = (paddr_t)buddy_alloc(0);
    if (phys)
    {
        pmm_refcounts[pmm_frame_idx(phys)] = 1;
    }
    return phys;
}

paddr_t pmm_alloc_frame_at_size(size_t size)
{
    if (size == 0 || size > MAX_SIZE)
    {
        return 0;
    }

    // Round size up to the nearest power of two, then find the order
    size_t alloc_size = PAGE_SIZE; // minimum is order 0 = 1 page
    uint32_t order = 0;

    while (alloc_size < size && order < BUDDY_MAX_ORDER - 1)
    {
        alloc_size <<= 1;
        order++;
    }

    return buddy_alloc(order);
}

paddr_t pmm_alloc_heap_frame()
{
    return buddy_alloc(2);
}

void pmm_free_frame(paddr_t addr)
{
    paddr_t phys_addr = PAGE_ALIGN_DOWN(addr);
    buddy_free(phys_addr, 0);
}
