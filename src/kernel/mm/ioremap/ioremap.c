/*
 * File: ioremap.c
 * File Created: 06 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ioremap.h"
#include "mm/memdefs.h"
#include "mm/mmu/mmu.h"

#define MODULE "IOREMAP"

vaddr_t mmio_bump = (vaddr_t)MEMORY_MMIO_VIRT_BASE;

vaddr_t ioremap(paddr_t phys, size_t size)
{
    vaddr_t virt = mmio_bump;
    log_info(MODULE, "mapping p%p to v%p", phys, virt);
    log_info(MODULE, "mapping %u bytes", size);
    
    mmu_map_region(&kernel_page, virt, phys, size, mmio_flags);

    mmio_bump += PAGE_ALIGN_UP(size);

    return virt;
}

void ioremap_init()
{
    mmio_bump = (vaddr_t)MEMORY_MMIO_VIRT_BASE;
}