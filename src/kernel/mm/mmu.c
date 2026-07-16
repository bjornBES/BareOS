/*
 * File: mmu.c
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/mmu/mmu.h"
#include "mm/pmm.h"
#include "mm/memdefs.h"
#include "kernel/mmu.h"
#include "kernel/memory.h"
#include <errno/errno.h>

#define MODULE "MMU"

static spinlock_t page_copy_lock = {0};

extern int process_user_page_fault(intr_frame_t *regs, mmu_fault_info *info);

int mmu_page_fault_handler(intr_frame_t *regs, mmu_fault_info *info)
{
    fprintf(VFS_FD_DEBUG, "Page Fault\n");
    int result = process_user_page_fault(regs, info);
    if (result != RETURN_GOOD)
    {
        log_info(MODULE, "now a kernel problem");

        log_debug(MODULE, "fault_addr at %p", info->fault_addr);

        return result;
        
        for (;;)
        {
            ;
        }
    }
    return result;
}

void mmu_copy_contents(paddr_t src, paddr_t dst)
{
    spinlock_acquire(&page_copy_lock);

    // map src and dst into scratch windows
    mmu_arch_map(&kernel_page, MEMORY_PHYS_COPY_SRC, src, kernel_data_flags);
    mmu_arch_map(&kernel_page, MEMORY_PHYS_COPY_DST, dst, kernel_data_flags);

    // flush TLB for both
    mmu_arch_flush_page(MEMORY_PHYS_COPY_SRC);
    mmu_arch_flush_page(MEMORY_PHYS_COPY_DST);

    memset((void *)MEMORY_PHYS_COPY_DST, 0, PAGE_SIZE);
    memcpy((void *)MEMORY_PHYS_COPY_DST, (void *)MEMORY_PHYS_COPY_SRC, PAGE_SIZE);

    // unmap scratch windows
    mmu_arch_unmap(&kernel_page, MEMORY_PHYS_COPY_SRC);
    mmu_arch_unmap(&kernel_page, MEMORY_PHYS_COPY_DST);

    spinlock_release(&page_copy_lock);
}

paddr_t mmu_alloc_and_map(page_table_t *table, vaddr_t virt, mmu_flags_t flags)
{
    vaddr_t _virt = virt;
    paddr_t phys = pmm_alloc_frame();
    if (phys == 0)
    {
        SET_ERROR(ENOMEM);
        return 0; // OOM
    }

    mmu_arch_map(table, _virt, phys, flags);
    return phys;
}

paddr_t mmu_alloc_and_map_region(page_table_t *table, vaddr_t virt, size_t size, mmu_flags_t flags)
{
    vaddr_t _virt = virt;
    paddr_t org_phys = pmm_alloc_frame();
    paddr_t phys = org_phys;

    // Round up to page boundary
    size_t pages = PAGE_ALIGN_UP(size) / PAGE_SIZE;

    for (size_t i = 0; i < pages; i++)
    {
        mmu_arch_map(table, _virt, phys, flags);
        _virt += PAGE_SIZE;
        phys += PAGE_SIZE;
        pmm_alloc_frame();
    }
    return org_phys;
}

paddr_t mmu_map_region(page_table_t *table, vaddr_t virt, paddr_t phys, size_t size, mmu_flags_t flags)
{
    vaddr_t _virt = virt;
    paddr_t _phys = phys;

    // Round up to page boundary
    size_t pages = PAGE_ALIGN_UP(size) / PAGE_SIZE;
    log_info(MODULE, "mapping region [v%p-v%p] to [p%p-p%p] size in pages is %u with %i to %p",
             _virt, _virt + pages * PAGE_SIZE,
             _phys, _phys + pages * PAGE_SIZE, pages, flags, table->page_dir);
    for (size_t i = 0; i < pages; i++)
    {
        mmu_arch_map(table, _virt, _phys, flags);
        mmu_arch_flush_page(_virt);
        _virt += PAGE_SIZE;
        _phys += PAGE_SIZE;
    }

    return phys;
}

void mmu_free_region(page_table_t *table, vaddr_t virt, size_t size)
{
    vaddr_t _virt = virt;
    // error things maybe?

    // Round up to page boundary
    size_t pages = PAGE_ALIGN_UP(size) / PAGE_SIZE;

    for (size_t i = 0; i < pages; i++)
    {
        mmu_arch_unmap(table, _virt);
        _virt += PAGE_SIZE;
    }
}
