/*
 * File: ioremap.c
 * File Created: 06 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ioremap.h"
#include "memory/memdefs.h"
#include "memory/paging/paging.h"

#define MODULE "IOREMAP"

virt_addr mmio_bump = MEMORY_MMIO_VIRT_BASE;

virt_addr ioremap(phys_addr phys, size_t size)
{
    virt_addr virt = mmio_bump;
    log_info(MODULE, "mapping p%p to v%p", phys, virt);
    log_info(MODULE, "mapping %u bytes", size);
    
    paging_map_region(kernel_page, virt, phys, size, mmio_flags);

    mmio_bump += ALIGN_2_UP(size, PAGE_SIZE);

    return virt;
}

void ioremap_init()
{
    mmio_bump = MEMORY_MMIO_VIRT_BASE;
}