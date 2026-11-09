/*
 * File: vma.c
 * File Created: 05 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/vma/vma.h"

#include "mm/pmm/pmm.h"

#include "mm/memdefs.h"

#include "asm/mmu_arch.h"

#include "assert/assert.h"

#include "lists/list.h"

#include "align.h"
#include "memory.h"

#define MODULE "vma"

INTERNAL status_t vma_shrink_down(vma_memory_t *mm, vma_t *vma)
{
    // ENTER_FUNC(MODULE, "%p, %p", mm, vma);
    if (vma->start >= vma->end)
    {
        log_warn(MODULE, "start (%p) >= end (%p)", vma->start, vma->end);
        return KERRNO_UNSUCCESS;
    }
    vaddr_t old_page = vma->start;
    bool is_present;
    status_t ret = 0;
    ret = mmu_arch_is_present(mm->page_directory, old_page, &is_present);
    if (ret == KERRNO_SUCCESSES && is_present)
    {
        paddr_t frame;
        ret = mmu_arch_unmap(mm->page_directory, old_page, &frame);
        if (ret)
        {
            return ret;
        }
        pmm_deref_frame(frame);
    }
    else
    {
        // return KERRNO_SUCCESSES;
    }

    vma->start = old_page + PAGE_SIZE;
    return KERRNO_SUCCESSES;
}

INTERNAL status_t vma_shrink_up(vma_memory_t *mm, vma_t *vma)
{
    // ENTER_FUNC(MODULE, "%p, %p", mm, vma);
    if (vma->end >= vma->start)
    {
        log_warn(MODULE, "start (%p) >= end (%p)", vma->end, vma->start);
        return KERRNO_UNSUCCESS;
    }
    status_t ret = 0;
    vaddr_t old_page = vma->end - PAGE_SIZE;
    bool is_present;
    ret = mmu_arch_is_present(mm->page_directory, old_page, &is_present);
    if (ret == KERRNO_SUCCESSES && is_present)
    {
        paddr_t frame;
        ret = mmu_arch_unmap(mm->page_directory, old_page, &frame);
        if (ret)
        {
            return ret;
        }
        pmm_deref_frame(frame);
    }
    else
    {
        return KERRNO_SUCCESSES;
    }

    vma->end = old_page;
    return KERRNO_SUCCESSES;
}

status_t vma_shrink(vma_memory_t *mm, vma_t *vma)
{
    ENTER_FUNC("%p, %p", mm, vma);
    if (vma->flags.grows_down)
    {
        return vma_shrink_down(mm, vma);
    }
    else
    {
        return vma_shrink_up(mm, vma);
    }
}

static bool vma_assert_no_overlap(vma_memory_t *mm, vma_t *new)
{
#ifdef DEBUG
    trace_debug(MODULE, "vma_assert_no_overlap");
    trace_debug(MODULE, "%p, next:%p", mm, mm->mmap);

    vma_t *cur = mm->mmap;
    while (cur)
    {
        if (cur != new)
        {
            trace_debug(MODULE, "cur: %p, next:%p", cur, cur->next);
            trace_debug(MODULE, "new: %p, next:%p", new, new->next);
            trace_debug(MODULE, "new->end = %p", new->end);
            trace_debug(MODULE, "new->start = %p", new->start);
            trace_debug(MODULE, "cur->end = %p", cur->end);
            trace_debug(MODULE, "cur->start = %p", cur->start);
            ASSERT_RETURN((new->end <= cur->start) || (new->start >= cur->end), false,
                          "VMA overlap: [%p-%p) vs [%p-%p)",
                          new->start, new->end,
                          cur->start, cur->end);
        }
        cur = cur->next;
    }
#endif
    return true;
}

// insert sorted by start
void vma_insert(vma_memory_t *mm, vma_t *new)
{
    trace_debug(MODULE, "vma_insert");
    if (vma_assert_no_overlap(mm, new) == false)
    {
        return;
    }

    // empty list
    if (mm->mmap == NULL)
    {
        new->prev = NULL;
        new->next = NULL;
        mm->mmap = new;
        mm->map_count++;
        return;
    }

    // find insertion point — list is sorted by start
    vma_t *cur = mm->mmap;
    while (cur->next && cur->next->start < new->start)
    {
        cur = cur->next;
    }

    // insert before head
    if (cur->start > new->start)
    {
        new->prev = NULL;
        new->next = cur;
        cur->prev = new;
        mm->mmap = new;
    }
    // insert after cur
    else
    {
        new->prev = cur;
        new->next = cur->next;
        if (cur->next)
        {
            cur->next->prev = new;
        }
        cur->next = new;
    }

    mm->map_count++;
    trace_debug(MODULE, "vma_insert DONE");
}

status_t do_mmap(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags, vma_t **out_vma)
{
    ENTER_FUNC("%p, %p, 0x%x, %u, 0x%x, %p", mm, addr, size, type, flags, out_vma);
    // page-align
    vaddr_t base = (vaddr_t)(addr & ~(PAGE_SIZE - 1));
    trace_debug(MODULE, "base = %p, addr = %p", base, addr);
    trace_debug(MODULE, "flags = %x", flags);
    size_t len = ALIGN_UP(size, PAGE_SIZE);

    // allocate and fill the vma
    vma_t *vma = kzmalloc(sizeof(vma_t));
    vma->start = base;
    vma->end = base + len;
    vma->top = base;
    vma->flags = flags;
    vma->type = type;
    vma->next = NULL;
    vma->prev = NULL;

    // demand paging

    vma_insert(mm, vma);
    *out_vma = vma;
    return KERRNO_SUCCESSES;
}

vma_t *do_mmap_eager(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags)
{
    ENTER_FUNC("%p, %p, 0x%x, %u, 0x%x", mm, addr, size, type, flags);
    page_table_t curr;
    mmu_arch_current_table(&curr);
    mmu_arch_load_table(mm->page_directory);
    // page-align
    vaddr_t base = PAGE_ALIGN_DOWN(addr);
    trace_debug(MODULE, "base = %p, addr = %p", base, addr);
    trace_debug(MODULE, "flags = 0x%x", flags);
    size_t len = PAGE_ALIGN_UP(size);

    // allocate and fill the vma
    vma_t *vma = kzmalloc(sizeof(vma_t));
    if (flags.grows_down)
    {
        vma->start = base - len;
        vma->end = base;
        vma->top = base - len;
    }
    else
    {
        vma->start = base;
        vma->end = base + len;
        vma->top = base;
    }
    vma->flags = flags;
    vma->type = type;

    bool state = mmu_arch_is_prints_disable();
    mmu_arch_enable_prints();
    // eagerly map pages (no demand paging yet)
    if (flags.grows_down)
    {
        for (vaddr_t va = base; base - len < va; va -= PAGE_SIZE)
        {
            paddr_t frame = pmm_alloc_frame();
            // virt_addr virt_frame = phys_to_virt_auto(frame);
            // log_debug(MODULE, "phys = %p | virt = %p", frame, virt_frame);
            trace_debug(MODULE, "mapping virt %p .. phys %p", va, frame);
            trace_debug(MODULE, "mm = %p, mm->page_directory = %p", mm, mm->page_directory);
            trace_debug(MODULE, "mm->page_directory->page_dir_phys = %p", mm->page_directory->page_dir_phys);
            mmu_arch_map(mm->page_directory, va, frame, flags, NULL);
            memset((void *)va, 0, PAGE_SIZE);
        }
    }
    else
    {
        for (vaddr_t va = base; va < base + len; va += PAGE_SIZE)
        {
            paddr_t frame = pmm_alloc_frame();
            // virt_addr virt_frame = phys_to_virt_auto(frame);
            // log_debug(MODULE, "phys = %p | virt = %p", frame, virt_frame);
            trace_debug(MODULE, "mapping virt %p .. phys %p", va, frame);
            trace_debug(MODULE, "mm = %p, mm->page_directory = %p", mm, mm->page_directory);
            trace_debug(MODULE, "mm->page_directory->page_dir_phys = %p", mm->page_directory->page_dir_phys);
            mmu_arch_map(mm->page_directory, va, frame, flags, NULL);
            memset((void *)va, 0, PAGE_SIZE);
        }
    }

    vma_insert(mm, vma);
    mmu_arch_set_prints(state);
    mmu_arch_load_table(&curr);
    return vma;
}

status_t vma_free(vma_memory_t *mm)
{
    ENTER_FUNC("%p", mm);
    vma_t *cur = mm->mmap;
    while (cur)
    {
        vma_t *current_map = cur;
        cur = cur->next;
        log_debug(MODULE, "starting %p", current_map);
        while (!vma_shrink(mm, current_map))
        {
        }
        log_debug(MODULE, "done here");
        kfree(current_map);
    }
    return KERRNO_SUCCESSES;
}

status_t vma_allocate(vma_memory_t **out_vma)
{
    vma_memory_t *vma_mm = kzmalloc(sizeof(vma_memory_t));
    *out_vma = vma_mm;
    mmu_arch_create_table(&vma_mm->page_directory);

    trace_info(MODULE, "mmap stack vma_mm = %p", vma_mm);
    vma_t *stack = do_mmap_eager(vma_mm, USER_STACK_TOP, PAGE_SIZE * 2, VMA_ANONYMOUS, stack_flags);
    trace_info(MODULE, "stack @ %p = {%p, %p}", stack, stack->start, stack->end);
    trace_info(MODULE, "mmap heap vma_mm = %p", vma_mm);
    vma_t *heap = do_mmap_eager(vma_mm, USER_HEAP_BASE, PAGE_SIZE * 2, VMA_ANONYMOUS, data_flags);

    vma_mm->start_stack = stack->end;
    vma_mm->start_heap = heap->start;
    vma_mm->end_heap = heap->end;
    vma_mm->heap_vma = heap;
    vma_mm->stack_vma = stack;
    return KERRNO_SUCCESSES;
}

status_t vma_clone(vma_memory_t *dest, vma_memory_t *src)
{
    status_t ret = 0;
    page_table_t curr;
    mmu_arch_current_table(&curr);
    mmu_arch_load_table(&kernel_page);

    dest->start_stack = src->start_stack;
    dest->start_heap = src->start_heap;
    dest->end_heap = src->end_heap;

    vma_t *current = src->mmap;
    while (current)
    {
        vma_t *new_vma = kmalloc(sizeof(vma_t));
        memcpy(new_vma, current, sizeof(vma_t));
        trace_info(MODULE, "new_vma = %p, dest = %p\n", new_vma, dest);
        size_t region_size = PAGE_ALIGN_UP(current->end - current->start);
        for (size_t offset = 0; offset < region_size; offset += PAGE_SIZE)
        {
            vaddr_t va = current->start + offset;
            paddr_t src_phys;
            ret = mmu_arch_virt_to_phys(&curr, va, &src_phys);
            if (ret)
            {
                continue;
            }

#if CONFIG_USE_EAGER_FORK == 1
            if (current->type == VMA_DEVICE || current->type == VMA_SHARED)
            {
                mmu_arch_map(proc->page_dir, va, src_phys, current->flags, NULL);
                continue;
            }
            paddr_t dst_phys = pmm_alloc_frame();
            mmu_copy_contents(src_phys, dst_phys);
            mmu_arch_map(proc->page_dir, va, dst_phys, current->flags, NULL);
#else // Cow
            mmu_flags_t new_flags = current->flags;
            new_flags.present = 1;
            new_flags.write = 0;
            new_flags.cow = 1;
            if (current->type == VMA_DEVICE || current->type == VMA_SHARED)
            {
                mmu_arch_map(dest->page_directory, va, src_phys, current->flags, NULL);
                continue;
            }

            if (current->type == VMA_ANONYMOUS)
            {
                if (current->end == USER_STACK_TOP && dest->stack_vma == NULL)
                {
                    dest->stack_vma = new_vma;
                    trace_debug(MODULE, "===== STACK HERE");
                }
                if (current->top == USER_HEAP_BASE && dest->heap_vma == NULL)
                {
                    dest->heap_vma = new_vma;
                    trace_debug(MODULE, "===== HEAP HERE");
                }
            }
            mmu_arch_map(dest->page_directory, va, src_phys, new_flags, NULL);

            mmu_arch_protect(src->page_directory, va, new_flags);

            pmm_ref_frame(src_phys);
#endif
        }
        trace_info(MODULE, "new_vma = %p [%p..%p] of type %u flags %x\n", new_vma, new_vma->start, new_vma->end, new_vma->type, new_vma->flags);
        trace_info(MODULE, "current = %p [%p..%p] of type %u flags %x\n", current, current->start, current->end, current->type, current->flags);
        vma_insert(dest, new_vma);
        current = current->next;
    }

    trace_debug(MODULE, "done cloning");
    return mmu_arch_load_table(&curr);
}
