/*
 * File: kstack_allocator.c
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kstack_allocator.h"
#include "memory/paging/paging.h"
#include "memory/pmm/pmm.h"

#define MODULE "kstack"

uint64_t stack_bump;

void kstack_init()
{
    stack_bump = (uint64_t)MEMORY_STACKS_VIRT_BASE;
}

void *kstack_alloc()
{
    // guard page sits at stack_bump, leave it unmapped
    virt_addr guard = (virt_addr)stack_bump;
    log_debug(MODULE, "guard = %p", guard);
    virt_addr stack_bottom = guard + PAGE_SIZE;
    log_debug(MODULE, "stack_bottom = %p", stack_bottom);
    virt_addr stack_top = guard + KERNEL_STACK_SIZE;
    log_debug(MODULE, "stack_top = %p", stack_top);

    // alloc physical frames and map them
    for (virt_addr va = stack_bottom; va < stack_top; va += PAGE_SIZE)
    {
        phys_addr pa = pmm_alloc_frame(); // order 0 = one page
        paging_map_page(kernel_page, va, pa, kernel_data_flags);
    }

    // guard page stays unmapped — any overflow hits it and page faults cleanly

    stack_bump += KERNEL_STACK_SLOT;

    return (void *)stack_top; // return TOP since stack grows down
}

void kstack_free(void *stack_top)
{
    virt_addr top = (virt_addr)stack_top;
    virt_addr stack_bottom = (top - KERNEL_STACK_SIZE) + PAGE_SIZE;
    log_debug(MODULE, "freeing %p bottom @ %p", stack_top, stack_bottom);

    phys_addr pa;
    for (virt_addr va = stack_bottom; va < top; va += PAGE_SIZE)
    {
        pa = (phys_addr)paging_get_physical(kernel_page, va);
        paging_unmap_page(kernel_page, va);
        pmm_free_frame(pa);
    }
}