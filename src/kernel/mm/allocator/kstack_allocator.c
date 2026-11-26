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

status_t stack_alloc(stack_info_t *info, size_t size, vaddr_t *out_addr)
{
    ENTER_FUNC("%p", info);
    // guard page sits at stack_bump, leave it unmapped
    vaddr_t guard = stack_bump;
    trace_debug(MODULE, "guard = %p", guard);
    vaddr_t stack_bottom = guard + PAGE_SIZE;
    trace_debug(MODULE, "stack_bottom = %p", stack_bottom);
    vaddr_t stack_top = guard + size;
    trace_debug(MODULE, "stack_top = %p", stack_top);

    // alloc physical frames and map them
    int count = 0;
    for (vaddr_t va = stack_bottom; va < stack_top; va += PAGE_SIZE)
    {
        paddr_t pa;
        status_t ret = pmm_alloc_frame(&pa);
        if (ret)
        {
            if (ret == KERRNO_OUT_OF_MEMORY)
            {
                KERNEL_PANIC(MODULE, "OOM");
            }
            LOG_RETURN(ret);
        }
        ret = mmu_arch_map(&kernel_page, va, pa, kernel_stack_flags, NULL);
        if (ret)
        {
            for (size_t i = 0; i < count; i++)
            {
                mmu_arch_unmap(&kernel_page, va, NULL);
            }
            LOG_RETURN(ret);
        }
        count++;
    }

    if (info != NULL)
    {
        info->mapping_flags = kernel_stack_flags;
        info->has_guard_page = 1;
        info->stack_bottom = stack_bottom;
        info->stack_size = size;
        info->stack_top = stack_top;
    }

    // guard page stays unmapped — any overflow hits it and page faults cleanly

    stack_bump += size;

    *out_addr = stack_top; // return TOP since stack grows down
    return KERRNO_SUCCESS;
}

status_t kstack_alloc(stack_info_t *info, vaddr_t *out_addr)
{
    ENTER_FUNC("%p", info);
    return stack_alloc(info, KERNEL_STACK_SIZE, out_addr);
}

status_t kstack_per_cpu_alloc(stack_info_t *info, vaddr_t *out_addr)
{
    ENTER_FUNC("%p", info);
    return stack_alloc(info, KERNEL_STACK_SLOT, out_addr);
}

void kstack_free(vaddr_t stack_top)
{
    vaddr_t top = (vaddr_t)stack_top;
    vaddr_t stack_bottom = (top - KERNEL_STACK_SIZE) + PAGE_SIZE;
    trace_debug(MODULE, "freeing %p bottom @ %p", stack_top, stack_bottom);

    paddr_t pa;
    for (vaddr_t va = stack_bottom; va < top; va += PAGE_SIZE)
    {
        mmu_arch_unmap(&kernel_page, va, &pa);
        pmm_free_frame(pa);
    }
}
