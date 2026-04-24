/*
 * File: pmm.c
 * File Created: 16 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pmm.h"
#include "debug/debug.h"

#include "memory/memdefs.h"
#include "memory/paging/paging.h"

#include <libs/memory.h>
#include <util/binary.h>

virt_addr phys_to_virt_auto(phys_addr p)
{
    // log_debug("PMM", "phys_to_virt_auto(%p) : is %p < %p then kernel else heap", p, p, start);
    if ((uint64_t)p < (uint64_t)start)
    {
        return (virt_addr)((uint64_t)p + (KERNEL_VIRT_BASE - KERNEL_PHYS_BASE));
    }
    else
    {
        return (virt_addr)((uint64_t)p + MEMORY_DIRECT_MAP_VIRT_BASE);
    }
}

void pmm_init(boot_params *params)
{
    buddy_init(params);
}

void pmm_map()
{
    buddy_map();
}

void pmm_print_info()
{
    buddy_print_info();
}
void pmm_print_info_verbose()
{
    buddy_print_info_verbose();
}

phys_addr pmm_alloc_region(phys_addr start_phys, phys_addr end_phys)
{
    phys_addr addr = (phys_addr)(uint32_64)ALIGN_DOWN((uint32_64)start_phys, PAGE_SIZE);
    phys_addr top  = (phys_addr)(uint32_64)ALIGN_UP((uint32_64)end_phys, PAGE_SIZE);
    while (addr < top)
    {
        buddy_alloc_at(addr);
        addr += PAGE_SIZE;
    }
    return addr;
}

phys_addr pmm_alloc_at(phys_addr addr)
{
    return buddy_alloc_at(addr);
    // not in any free list — already allocated, nothing to do
}

phys_addr pmm_alloc_frame()
{
    return buddy_alloc(0);
}
phys_addr pmm_alloc_heap_frame()
{
    return buddy_alloc(2);
}

void pmm_free_frame(phys_addr addr)
{
    buddy_free(addr, 0);
}
