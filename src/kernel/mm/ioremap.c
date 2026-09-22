/*
 * File: ioremap.c
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/ioremap.h"
#include "mm/memdefs.h"
#include "mm/mmu.h"
#include "asm/mmu_arch.h"
#include "debug/debug.h"

#define MODULE "IOREMAP"

#define MODULE "IOREMAP"

vaddr_t mmio_bump = (vaddr_t)MEMORY_MMIO_VIRT_BASE;

void ioremap_init()
{
    mmio_bump = (vaddr_t)MEMORY_MMIO_VIRT_BASE;
}

vaddr_t ioremap(paddr_t phys, size_t size)
{
    vaddr_t virt = mmio_bump;

    if (!mmu_arch_is_prints_disable())
    {
        trace_info(MODULE, "mapping p%p to v%p", phys, virt);
        trace_info(MODULE, "mapping %u bytes", size);
    }

    mmu_map_region(&kernel_page, virt, phys, size, mmio_flags);

    mmio_bump += PAGE_ALIGN_UP(size);

    return virt;
}
