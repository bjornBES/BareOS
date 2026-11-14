/*
 * File: dma.c
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/dma/dma.h"
#include "mm/pmm/pmm.h"
#include "mm/memdefs.h"

#include "asm/dma_arch.h"

#include "align.h"
#include "memory.h"
#include "init.h"

#include "kerrno.h"

#define MODULE "dma"

INTERNAL vaddr_t dma_bump = MEMORY_DMA_VIRT_BASE;

status_t dma_initialize()
{
    dma_bump = MEMORY_DMA_VIRT_BASE;
    return KERRNO_SUCCESSES;
}

EARLY_INITCALL(dma_initialize);

INTERNAL dma_t *dma_create()
{
    return (dma_t *)kzmalloc(sizeof(dma_t));
}

INTERNAL vaddr_t dma_get_address(size_t size)
{
    vaddr_t result = dma_bump;
    dma_bump += (vaddr_t)size;
    return result;
}

status_t dma_allocate(size_t size, dma_t **out_dma)
{
    status_t ret;
    size_t aligned_size = PAGE_ALIGN_UP(size);
    paddr_t addr;
    ret = pmm_alloc_frame_at_size(FRAME_COUNT(aligned_size), &addr); // allocates the biggest single frame
    if (ret)
    {
        KERRNO_RETURN(KERRNO_OOM, "");
    }
    dma_t *dma = dma_create();
    *out_dma = dma;
    dma->phys = addr;
    dma->size = aligned_size;
    dma->virt = dma_get_address(aligned_size);
    return dma_arch_map(dma, aligned_size); // mapping the dma region by looking at the aligned_size
}

status_t dma_copy_from(dma_t *alloc, void *src, size_t size, size_t *out_size)
{
    // ENTER_FUNC(MODULE, "%p, %p, %u", alloc, src, size);
    size_t ret_size;
    status_t ret = dma_arch_copy_from(alloc, src, size, &ret_size);
    if (out_size)
    {
        *out_size = ret_size;
    }
    return ret;
}

status_t dma_copy_to(dma_t *alloc, void *dst, size_t size, size_t *out_size)
{
    // ENTER_FUNC(MODULE, "%p, %p, %u", alloc, dst, size);
    size_t ret_size;
    status_t ret = dma_arch_copy_to(alloc, dst, size, &ret_size);
    if (out_size)
    {
        *out_size = ret_size;
    }
    return ret;
}
