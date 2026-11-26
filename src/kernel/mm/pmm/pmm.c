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

#include "resource/resource.h"

#include "init.h"
#include "math.h"

#define MODULE "pmm"

pmm_info_t pmm_info;

pmm_zone_info_t *bump_info;
static paddr_t bump_base;
static paddr_t bump_current;
static paddr_t bump_end;
static bool pmm_ready = false;

vaddr_t kernel_virt_base;
vaddr_t kernel_virt_end;

paddr_t kernel_phys_base;
paddr_t kernel_phys_end;

size_t mem_size;

pmm_zone_info_t infos[CONFIG_MAX_INFOS];
int infos_count = 0;
pmm_zone_info_t *kernel_info;
pmm_zone_info_t *main_info;

extern char pmm_early_heap_size;

frame_allocator_t *registry[CONFIG_MAX_FRAME_ALLOCATORS];
int registry_index = 0;

uint16_t *pmm_refcounts = NULL;
size_t pmm_frame_count = 0;

ALWAYS_INLINE void pmm_print_info_entry(pmm_zone_info_t *info)
{
    trace_info(MODULE, "info   %p ", info);
    trace_info(MODULE, "info   %p phys_start=%016x, phys_end=%016x, zone=%u, virt_start=%016x, virt_end=%016x", info, info->phys_start, info->phys_end, info->zone, info->virt_start, info->virt_end);
}

ALWAYS_INLINE void pmm_print_entry(memory_entry_t *entry)
{
    trace_info(MODULE, "region %p      start=%016x, size =   %016x, type=%u", entry, entry->addr, entry->size, entry->type);
}

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

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_ALLOCATOR);

    memory_entry_t *entry = NULL;
    kernel_phys_end = bp->kernel_phys_base + bp->kernel_size;
    kernel_virt_base = bp->kernel_virt_base;
    kernel_phys_base = bp->kernel_phys_base;

    trace_debug(MODULE, "pmm_init(%p)", bp);
    trace_debug(MODULE, "count = %u entries =%p", entry_count, memory);
    for (size_t i = 0; i < entry_count; i++)
    {
        entry = &memory[i];
        pmm_print_entry(entry);
    }
    paddr_t split;
    pmm_zone_info_t *info;
    for (size_t i = 0; i < entry_count; i++)
    {
        entry = &memory[i];

        if (entry->type != MEMORY_AVAILABLE)
        {
            continue;
        }

        info = &infos[infos_count];

        info->phys_start = (paddr_t)PAGE_ALIGN_UP((paddr_t)entry->addr);
        info->phys_end = (paddr_t)PAGE_ALIGN_UP((paddr_t)(entry->addr + entry->size));
        info->memory = entry;

        status_t ret = mmu_arch_get_zone(info->phys_start, info->phys_end, &split, &info->zone);
        if (ret == KERRNO_BAD_PADDR)
        {
            size_t diff = info->phys_start - split;

            memory_entry_t *next_entry = &memory[i + 1];
            int remaining_entries = entry_count - i;
            for (size_t j = remaining_entries - 1; j > i; j--)
            {
                memcpy(&memory[j + 1], &memory[j], sizeof(memory_entry_t));
            }
            next_entry->addr = split;
            next_entry->type = memory[i + 2].type;
            next_entry->type = memory[i + 2].ext_data;
            next_entry->size = diff;
            entry->size -= diff;
            entry_count++;
            i--;
            continue;
        }

        pmm_print_entry(entry);
        if (entry->addr >= 0x100000)
        {
            if (entry->addr < (paddr_t)kernel_phys_end)
            {
                info->phys_start = (paddr_t)PAGE_ALIGN_UP((paddr_t)kernel_phys_end);
            }
        }

        pmm_print_info_entry(info);

        if (info->phys_start >= kernel_phys_end)
        {
            kernel_info = info;
            trace_info(MODULE, "early_size = %p", early_size);
            if (entry->size > early_size && entry->addr >= 0x100000)
            {
                bump_base = info->phys_start;
                bump_current = bump_base;
                bump_end = bump_base + early_size;
                bump_info = info;
                info->phys_start = bump_end + PAGE_SIZE;
                info->phys_start = ALIGN_2_UP(info->phys_start, 0x200000);
                trace_debug(MODULE, "bump_current = %p bump_end = %p", bump_current, bump_end);
            }
        }
        if (main_info)
        {
            size_t size = max(main_info->phys_end - main_info->phys_start, entry->size);
            if (size == entry->size)
            {
                main_info = info;
            }
        }
        else
        {
            main_info = info;
        }
        pmm_print_entry(entry);
        pmm_print_info_entry(info);
        infos_count++;
    }
    for (size_t i = 0; i < entry_count; i++)
    {
        entry = &memory[i];
        if (entry->type != MEMORY_AVAILABLE)
        {
            continue;
        }
        pmm_print_entry(entry);
        mem_size += entry->size;
    }
    for (size_t i = 0; i < infos_count; i++)
    {
        info = &infos[i];
        pmm_print_info_entry(info);
    }
    mem_size = mem_size;

    trace_debug(MODULE, "bump_current = %p bump_end = %p", bump_current, bump_end);

    vaddr_t start_vaddr = MEMORY_DIRECT_MAP_VIRT_BASE;
    for (size_t i = 0; i < infos_count; i++)
    {
        info = &infos[i];
        size_t size = info->phys_end - info->phys_start;
        info->virt_start = start_vaddr;
        info->virt_end = start_vaddr + size;
        trace_debug(MODULE, "info.virt_start=%p", info->virt_start);
        start_vaddr += size;
        for (size_t j = 0; j < registry_index; j++)
        {
            frame_allocator_t *allocator = registry[j];
            status_t ret = allocator->setup(&info->allocator_info, info);
            if (ret)
            {
                continue;
            }
            info->allocator = allocator;
            trace_info(MODULE, "info->allocator = %p", info->allocator);
            trace_info(MODULE, "info->allocator @ %p", &info->allocator);
        }
        pmm_print_info_entry(info);
        pmm_print_entry(info->memory);
    }

    return KERRNO_SUCCESS;
}

status_t pmm_init()
{
    ENTER_FUNC("", 0);
    // pmm_reg_initialize(&info, registry);

    for (size_t i = 0; i < infos_count; i++)
    {
        pmm_zone_info_t *info = &infos[i];
        trace_info(MODULE, "info @ %p", info);
        pmm_print_info_entry(info);
        pmm_print_entry(info->memory);
        // trace_info(MODULE, "start        = %p end      = %p", info->phys_start, info->phys_end);
        trace_info(MODULE, "info->allocator = %p", info->allocator);
        trace_info(MODULE, "info->allocator @ %p", &info->allocator);
        // trace_info(MODULE, "info->allocator->initialize = %p", info->allocator->initialize);
        // trace_info(MODULE, "info->allocator->initialize @ %p", &info->allocator->initialize);
        if ((void *)info->allocator->initialize < (void *)0x1000)
        {
            continue;
        }
        info->allocator->initialize(&info->allocator_info, info);
        info->allocator->ops.print_stat_verbose(&info->allocator_info);
    }

    pmm_frame_count = mem_size / PAGE_SIZE;
    trace_debug(MODULE, "pmm_frame_count = %u", pmm_frame_count);
    paddr_t pmm_refcounts_addr = 0;
    status_t ret = pmm_alloc_frames_contiguous(PAGE_ALIGN_UP(pmm_frame_count * sizeof(uint16_t)) / PAGE_SIZE, &pmm_refcounts_addr);
    if (ret)
    {
        KERNEL_PANIC(MODULE, "early heap is exhausted");
    }
    pmm_refcounts = (uint16_t *)phys_to_virt_auto(pmm_refcounts_addr);
    memset(pmm_refcounts, 0, pmm_frame_count * sizeof(uint16_t));

    pmm_ready = true;

    return KERRNO_SUCCESS;
}

CORE_INITCALL(pmm_init);

status_t pmm_register_allocator(frame_allocator_t *allocator)
{
    registry[registry_index] = allocator;
    registry_index++;
    return KERRNO_SUCCESS;
}

status_t pmm_insert_resource(boot_params_t *params)
{
    memory_entry_t *memory = params->memory.entries;
    uint32_t entry_count = params->memory.count;

    for (size_t i = 0; i < entry_count; i++)
    {
        memory_entry_t *entry = &memory[i];
        resource_t *node = kmalloc(sizeof(resource_t));
        const char *name;
        res_type_t type;
        switch (entry->type)
        {
            case MEMORY_AVAILABLE :
                name = "System RAM";
                type = RES_TYPE_RAM;
                break;
            case MEMORY_RESERVED :
            case MEMORY_ACPI_RECLAIMABLE :
            case MEMORY_ACPI_NVS :
            case MEMORY_BAD_RAM :
                name = "Reserved";
                type = RES_TYPE_RESERVED;
                break;

            default :
                break;
        }
        if (resource_request(&mem_space.root, node, entry->addr, entry->addr + entry->size, name, type, RES_FLAG_NONE) != KERRNO_SUCCESS)
        {
            KERNEL_PANIC(MODULE, "something is wrong");
        }
    }

    return KERRNO_SUCCESS;
}

INTERNAL pmm_zone_info_t *get_zone_from_paddr(paddr_t addr)
{
    // ENTER_FUNC("%p", addr);
    for (size_t i = 0; i < infos_count; i++)
    {
        pmm_zone_info_t *info = &infos[i];
        // trace_debug(MODULE, "%p >= %p && %p < %p", addr, info->phys_start, addr, info->phys_end);
        if (addr >= info->phys_start && addr < info->phys_end)
        {
            // trace_debug(MODULE, "start        = 0x%016llx end      = 0x%016llx", info->phys_start, info->phys_end);
            return &infos[i];
        }
    }
    return NULL;
}

INTERNAL pmm_zone_info_t *get_zone_from_vaddr(vaddr_t addr)
{
    // ENTER_FUNC("%p", addr);
    for (size_t i = 0; i < infos_count; i++)
    {
        pmm_zone_info_t *info = &infos[i];
        // trace_debug(MODULE, "%p >= %p && %p < %p", addr, info->virt_start, addr, info->virt_end);
        if (addr >= info->virt_start && addr < info->virt_end)
        {
            // trace_debug(MODULE, "start        = 0x%016llx end      = 0x%016llx", info->phys_start, info->phys_end);
            return info;
        }
    }
    return NULL;
}

INTERNAL_INLINE status_t bump_alloc(paddr_t *out_addr)
{
    if (bump_current >= bump_end)
    {
        KERRNO_RETURN(KERRNO_OUT_OF_MEMORY, "early heap exhausted");
    }
    *out_addr = bump_current;
    bump_current += PAGE_SIZE;
    return KERRNO_SUCCESS;
}

INTERNAL_INLINE size_t pmm_frame_idx(paddr_t phys)
{
    return phys >> 12;
}

status_t pmm_alloc_frame(paddr_t *out_addr)
{
    status_t ret;
    paddr_t phys;
    if (!pmm_ready)
    {
        // trace_debug(MODULE, "Allocating from bump %p/%p, %p", bump_current, bump_end, &pmm_ready);
        ret = bump_alloc(&phys);
        if (ret)
        {
            LOG_RETURN(ret);
        }
    }
    else
    {
        ret = main_info->allocator->ops.alloc(&main_info->allocator_info, 1, &phys);
        if (ret)
        {
            LOG_RETURN(ret);
        }
        if (phys)
        {
            pmm_refcounts[pmm_frame_idx(phys)] = 1;
        }
    }
    trace_debug(MODULE, "got addr=%p", phys);
    if (out_addr)
    {
        *out_addr = phys;
    }
    return KERRNO_SUCCESS;
}

status_t pmm_free_frame(paddr_t addr)
{
    paddr_t phys_addr = PAGE_ALIGN_DOWN(addr);
    return main_info->allocator->ops.free(&main_info->allocator_info, phys_addr, 0);
}

status_t pmm_alloc_frames_contiguous(size_t times, paddr_t *out_addr)
{
    paddr_t base;
    status_t ret = pmm_alloc_frame(&base);
    if (ret)
    {
        LOG_RETURN(ret);
    }
    for (size_t i = 0; i < (times - 1); i++)
    {
        ret = pmm_alloc_frame(NULL);
        if (ret)
        {
            LOG_RETURN(ret);
        }
    }
    if (out_addr)
    {
        *out_addr = base;
    }
    return KERRNO_SUCCESS;
}

status_t pmm_alloc_frame_at_size(size_t nframes, paddr_t *out_addr)
{
    if (nframes == 0)
    {
        return 0;
    }

    paddr_t addr;
    status_t ret = main_info->allocator->ops.alloc(&main_info->allocator_info, nframes, &addr);
    *out_addr = addr;
    return ret;
}

status_t pmm_ref_frame(paddr_t phys)
{
    trace_debug(MODULE, "phys = %p", phys);
    size_t idx = pmm_frame_idx(phys);
    ASSERT(idx < pmm_frame_count, "idx = %u, pmm_frame_count = %u", idx, pmm_frame_count);
    pmm_refcounts[idx]++;
    return KERRNO_SUCCESS;
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
    return KERRNO_SUCCESS;
}

uint16_t pmm_get_refcount(paddr_t phys)
{
    return pmm_refcounts[pmm_frame_idx(phys)];
}

pmm_zone_info_t *pmm_get_kernel_zone_info()
{
    return kernel_info;
}

// pmm_info_t *pmm_get_info()
// {
//     return &info;
// }

void pmm_print_info()
{
    main_info->allocator->ops.print_stat(&main_info->allocator_info);
}

void pmm_print_info_verbose()
{
    // pmm_print_entry(main_info->memory);
    pmm_print_info_entry(main_info);
    main_info->allocator->ops.print_stat_verbose(&main_info->allocator_info);
}

vaddr_t phys_to_virt_auto(paddr_t p)
{
    // ENTER_FUNC("%p", p);
    vaddr_t result = 0;
    pmm_zone_info_t *info = get_zone_from_paddr(p);
    if (!info || p < info->phys_start)
    {
        result = (vaddr_t)(p + (kernel_virt_base - kernel_phys_base));
    }
    else
    {
        // trace_debug(MODULE, "start        = 0x%016llx end      = 0x%016llx", info->phys_start, info->phys_end);
        // trace_debug(MODULE, "p(%p) + (HEAP_VADDR_OFFSET - offset(%p)) = %p", p, info->phys_start, p + (MEMORY_DIRECT_MAP_VIRT_BASE - info->phys_start));
        result = (vaddr_t)(p + (MEMORY_DIRECT_MAP_VIRT_BASE - info->phys_start));
        // trace_debug(MODULE, "result = %p", result);
    }
    // trace_debug(MODULE, "result = %p", result);
    return result;
}

paddr_t virt_to_phys_auto(vaddr_t p)
{
    // trace_debug(MODULE, "virt_to_phys_auto(%p) : is %p >= %p then heap else kernel", p, p, kernel_virt_base);
    if (p >= kernel_virt_base)
    {
        // trace_debug(MODULE, "KERNEL p(%p) - offset(%p) = %p", p, kernel_virt_base + kernel_phys_base, p - kernel_virt_base + kernel_phys_base);
        return (paddr_t)(p - (kernel_virt_base - kernel_phys_base));
    }
    else
    {
        pmm_zone_info_t *info = get_zone_from_vaddr(p);
        // trace_debug(MODULE, "p(%p) - (HEAP_VADDR_OFFSET - offset(%p)) = %p", p, info->phys_start, p - (MEMORY_DIRECT_MAP_VIRT_BASE - info->phys_start));
        return (paddr_t)(p - (MEMORY_DIRECT_MAP_VIRT_BASE - info->phys_start));
    }
}
