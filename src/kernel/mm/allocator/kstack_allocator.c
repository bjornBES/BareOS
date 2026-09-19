/*
 * File: kstack_allocator.c
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/allocator/kstack_allocator.h"
#include "mm/pmm/pmm.h"
#include "mm/memdefs.h"

#include "stack_info/stack_info.h"

#include "asm/mmu_arch.h"
#include "asm/page_arch.h"

#include "debug/debug.h"

#define MODULE "kstack_allocator"

vaddr_t stack_bump = MEMORY_STACKS_VIRT_BASE;
vaddr_t cpu_stack_bump = MEMORY_CPU_STACKS_VIRT_BASE;

vaddr_t kstack_alloc(stack_info_t *info)
{
    // guard page sits at stack_bump, leave it unmapped
    vaddr_t guard = stack_bump;
    log_debug(MODULE, "guard = %p", guard);
    vaddr_t stack_bottom = guard + PAGE_SIZE;
    log_debug(MODULE, "stack_bottom = %p", stack_bottom);
    vaddr_t stack_top = guard + KERNEL_STACK_SIZE;
    log_debug(MODULE, "stack_top = %p", stack_top);

    // alloc physical frames and map them
    for (vaddr_t va = stack_bottom; va < stack_top; va += PAGE_SIZE)
    {
        paddr_t pa = pmm_alloc_frame();
        mmu_arch_map(&kernel_page, va, pa, kernel_stack_flags);
    }

    if (info != NULL)
    {
        info->mapping_flags = kernel_stack_flags;
        info->has_guard_page = 1;
        info->stack_bottom = stack_bottom;
        info->stack_size = KERNEL_STACK_SLOT;
        info->stack_top = stack_top;
    }

    // guard page stays unmapped — any overflow hits it and page faults cleanly

    stack_bump += KERNEL_STACK_SLOT;

    return stack_top; // return TOP since stack grows down
}

vaddr_t kstack_per_cpu_alloc(stack_info_t *info)
{
    ENTER_FUNC(MODULE, "", "");
    // guard page sits at cpu_stack_bump, leave it unmapped
    vaddr_t guard = cpu_stack_bump;
    log_debug(MODULE, "guard = %p", guard);
    vaddr_t stack_bottom = guard + PAGE_SIZE;
    log_debug(MODULE, "stack_bottom = %p", stack_bottom);
    vaddr_t stack_top = guard + (PAGE_SIZE * 4);
    log_debug(MODULE, "stack_top = %p", stack_top);

    // alloc physical frames and map them
    for (vaddr_t va = stack_bottom; va < stack_top; va += PAGE_SIZE)
    {
        paddr_t pa = pmm_alloc_frame();
        mmu_arch_map(&kernel_page, va, pa, kernel_stack_flags);
    }

    if (info != NULL)
    {
        info->mapping_flags = kernel_stack_flags;
        info->has_guard_page = 1;
        info->stack_bottom = stack_bottom;
        info->stack_size = KERNEL_STACK_SLOT;
        info->stack_top = stack_top;
    }

    // guard page stays unmapped — any overflow hits it and page faults cleanly

    cpu_stack_bump += KERNEL_STACK_SLOT;

    return stack_top; // return TOP since stack grows down
}

void kstack_free(vaddr_t stack_top)
{
    vaddr_t top = (vaddr_t)stack_top;
    vaddr_t stack_bottom = (top - KERNEL_STACK_SIZE) + PAGE_SIZE;
    log_debug(MODULE, "freeing %p bottom @ %p", stack_top, stack_bottom);

    paddr_t pa;
    for (vaddr_t va = stack_bottom; va < top; va += PAGE_SIZE)
    {
        // pa = (paddr_t)mmu_arch_virt_to_phys(&kernel_page, va);
        // mmu_arch_unmap(&kernel_page, va);
        // pmm_free_frame(pa);
    }
}
