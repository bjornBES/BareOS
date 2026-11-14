/*
 * File: dma.c
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/dma_arch.h"
#include "asm/mmu_arch.h"

#include "kernel/paging/paging.h"

#include "mm/mmut.h"
#include "mm/pmm/pmm.h"

#include "debug/debug.h"

#include "memory.h"

#define MODULE "x86-dma"

status_t dma_arch_map(dma_t *dma, size_t size)
{
    ENTER_FUNC("%p, %u", dma, size);
    mmu_flags_t flags = {0};
    flags.present = 1;
    flags.write = 1;
    if (size >= PAGE_SIZE_LARGE)
    {
        flags.large = 1;
    }
    dma->pages = size / PAGE_SIZE;
    vaddr_t vaddr = dma->virt;
    paddr_t paddr = dma->phys;
    for (size_t i = 0; i < dma->pages;)
    {
        size_t size;
        status_t ret = allocate_leaf(&kernel_page, vaddr, paddr, flags, &size);
        if (ret)
        {
            pmm_print_info_verbose();
            trace_err(MODULE, "failed to map region [%p - %p]", vaddr, vaddr + dma->size);
            KERNEL_PANIC(MODULE, "TODO"); // TODO: handle this
        }
        vaddr += size;
        paddr += size;
        i += PAGE_COUNT(size);
    }
    return KERRNO_SUCCESSES;
}

INTERNAL_INLINE size_t dma_do_copy(void *dest, void *src, size_t size)
{
    /*     if ((size % 8) == 0)
        {
            memcpy64_fast(dst, src, size/8);
        }
        else if ((size % 4) == 0)
        {
            memcpy32_fast(dst, src, size/4);
        }
        else if ((size % 2) == 0)
        {
            memcpy16_fast(dst, src, size/2);
        }
        else */
    {
        memcpy(dest, src, size);
    }
    return size;
}

status_t dma_arch_copy_from(dma_t *alloc, void *src, size_t size, size_t *out_size)
{
    *out_size = dma_do_copy((void *)alloc->virt, src, size);
    return KERRNO_SUCCESSES;
}

status_t dma_arch_copy_to(dma_t *alloc, void *dst, size_t size, size_t *out_size)
{
    ENTER_FUNC("%p, %p, %u, %p", alloc, dst, size, out_size);
    *out_size = dma_do_copy(dst, (void *)alloc->virt, size);
    return KERRNO_SUCCESSES;
}
