/*
 * File: vmm.c
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vmm.h"
#include "task/assert/assert.h"
#include "mm/memdefs.h"
#include "mm/pmm.h"
#include "mm/mmu/mmu.h"
#include "kernel/memory.h"
#include "mm/allocator/memory_allocator.h"
#include "debug/debug.h"

#define MODULE "VMA"

// find the VMA containing addr — the #PF handler's first call
vma_t *vma_find(vma_memory_t *mm, vaddr_t addr)
{
    vma_t *vma = mm->mmap;
    while (vma)
    {
        if (addr >= vma->start && addr < vma->end)
        {
            return vma;
        }
        vma = vma->next;
    }
    return NULL;
}

// find the VMA containing addr — the #PF handler's first call
int vma_inside(vma_memory_t *mm, vaddr_t addr)
{
    vma_t *vma = mm->mmap;
    while (vma)
    {
        if (addr >= vma->start && addr < vma->end)
        {
            return RETURN_GOOD;
        }
        vma = vma->next;
    }
    return RETURN_FAILED;
}

static inline int vma_grow_down(vma_memory_t *mm, vma_t *vma)
{
    vaddr_t new_page = vma->start - PAGE_SIZE;

    // make sure we don't collide with the vma below us
    // (the one with the highest start that is still < our start)
    if (vma->prev && vma->prev->end > new_page)
    {
        return RETURN_FAILED;
    }

    // paddr_t frame = pmm_alloc_frame();
    // mmu_arch_map(mm->page_directory, new_page, frame, vma->flags);
    // memset((void *)new_page, 0, PAGE_SIZE);

    vma->start = new_page;
    // no re-insert needed — start moved down but relative order in the
    // list hasn't changed (prev->end is still <= our new start)
    return RETURN_GOOD;
}

static inline int vma_grow_up(vma_memory_t *mm, vma_t *vma)
{
    vaddr_t new_page = vma->end;

    if (vma->next && vma->next->start < new_page + PAGE_SIZE)
    {
        return RETURN_FAILED;
    }

    // paddr_t frame = pmm_alloc_frame();
    // mmu_arch_map(mm->page_directory, new_page, frame, vma->flags);
    // memset((void *)new_page, 0, PAGE_SIZE);

    vma->end += PAGE_SIZE;
    return RETURN_GOOD;
}

static inline int vma_shrink_down(vma_memory_t *mm, vma_t *vma)
{
    // ENTER_FUNC(MODULE, "%p, %p", mm, vma);
    if (vma->start >= vma->end)
    {
        log_warn(MODULE, "start (%p) >= end (%p)", vma->start, vma->end);
        return RETURN_FAILED;
    }
    vaddr_t old_page = vma->start;
    if (mmu_arch_is_present(mm->page_directory, old_page) == RETURN_GOOD)
    {
        paddr_t frame = mmu_arch_unmap(mm->page_directory, old_page);
        if (frame == 0)
        {
            return RETURN_FAILED;
        }
        pmm_deref_frame(frame);
    }
    else
    {
        // return RETURN_FAILED;
    }

    vma->start = old_page + PAGE_SIZE;
    return RETURN_GOOD;
}

static inline int vma_shrink_up(vma_memory_t *mm, vma_t *vma)
{
    // ENTER_FUNC(MODULE, "%p, %p", mm, vma);
    if (vma->end >= vma->start)
    {
        log_warn(MODULE, "start (%p) >= end (%p)", vma->end, vma->start);
        return RETURN_FAILED;
    }
    vaddr_t old_page = vma->end - PAGE_SIZE;
    if (mmu_arch_is_present(mm->page_directory, old_page) == RETURN_GOOD)
    {
        paddr_t frame = mmu_arch_unmap(mm->page_directory, old_page);
        if (frame == 0)
        {
            return RETURN_FAILED;
        }
        pmm_deref_frame(frame);
    }
    else
    {
        return RETURN_FAILED;
    }

    vma->end = old_page;
    return RETURN_GOOD;
}

int vma_grow(vma_memory_t *mm, vma_t *vma)
{
    if (vma->flags.grows_down)
    {
        return vma_grow_down(mm, vma);
    }
    else
    {
        return vma_grow_up(mm, vma);
    }
}

int vma_shrink(vma_memory_t *mm, vma_t *vma)
{
    ENTER_FUNC(MODULE, "%p, %p", mm, vma);
    if (vma->flags.grows_down)
    {
        return vma_shrink_down(mm, vma);
    }
    else
    {
        return vma_shrink_up(mm, vma);
    }
}

void vma_free(process_t *proc)
{
    ENTER_FUNC(MODULE, "%p", proc);
    vma_memory_t *mm = proc->vma;
    vma_t *cur = mm->mmap;
    while (cur)
    {
        vma_t *current_map = cur;
        cur = cur->next;
        log_debug(MODULE, "starting %p", current_map);
        while (vma_shrink(mm, current_map) == RETURN_GOOD)
        {
        }
        log_debug(MODULE, "done here");
        free(current_map);
    }
}

void vma_destroy(process_t *proc)
{
    ENTER_FUNC(MODULE, "%p", proc);
    vma_memory_t *mm = proc->vma;
    vma_t *cur = mm->mmap;
    while (cur)
    {
        vma_t *current_map = cur;
        cur = cur->next;
        while (vma_shrink(mm, current_map) == RETURN_GOOD)
        {
        }
        free(current_map);
    }
}

#ifdef DEBUG
static void vma_assert_no_overlap(vma_memory_t *mm, vma_t *new)
{
    log_debug(MODULE, "vma_assert_no_overlap");
    fprintf(VFS_FD_DEBUG, "%p, next:%p\n", mm, mm->mmap);

    vma_t *cur = mm->mmap;
    while (cur)
    {
        if (cur != new)
        {
            fprintf(VFS_FD_DEBUG, "%p, next:%p\n", cur, cur->next);
            fprintf(VFS_FD_DEBUG, "%p, next:%p\n", new, new->next);
            ASSERT((new->end <= cur->start) || (new->start >= cur->end),
                   "VMA overlap: [%p-%p) vs [%p-%p)",
                   new->start, new->end,
                   cur->start, cur->end);
        }
        cur = cur->next;
    }
}
#endif

vma_t *vma_split(vma_memory_t *mm, vma_t *vma, vaddr_t split_addr)
{
    ENTER_FUNC(MODULE, "%p, %p, %p", mm, vma, split_addr);

    if (split_addr <= vma->start || split_addr >= vma->end)
    {
        log_err(MODULE, "vma_split: split_addr not strictly inside vma");
        return NULL;
    }

    vma_t *new_vma = kmalloc(sizeof(vma_t));
    memset(new_vma, 0, sizeof(vma_t));

    new_vma->start = split_addr;
    new_vma->end = vma->end;
    new_vma->top = vma->top > split_addr ? vma->top : split_addr;
    new_vma->flags = vma->flags;
    new_vma->type = vma->type;

    // shrink original to the head half
    vma->end = split_addr;
    if (vma->top > split_addr)
    {
        vma->top = split_addr;
    }

    // splice new_vma in right after vma in the sorted list
    new_vma->prev = vma;
    new_vma->next = vma->next;
    if (vma->next)
    {
        vma->next->prev = new_vma;
    }
    vma->next = new_vma;

    return new_vma;
}

// insert sorted by start
void vma_insert(vma_memory_t *mm, vma_t *new)
{
    log_debug(MODULE, "vma_insert");
    vma_assert_no_overlap(mm, new);

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
}

vma_memory_t *vma_clone(process_t *parent, process_t *proc)
{
    vma_memory_t *src = parent->vma;
    page_table_t curr;
    mmu_arch_current_table(&curr);
    mmu_arch_load_table(&kernel_page);

    vma_memory_t *new_memory = kmalloc(sizeof(vma_memory_t));
    memset(new_memory, 0, sizeof(vma_memory_t));
    new_memory->start_stack = src->start_stack;
    new_memory->start_heap = src->start_heap;
    new_memory->end_heap = src->end_heap;

    vma_t *current = src->mmap;
    while (current)
    {
        vma_t *new_vma = kmalloc(sizeof(vma_t));
        memcpy(new_vma, current, sizeof(vma_t));
        fprintf(VFS_FD_DEBUG, "new_vma = %p, new_memory = %p\n", new_vma, new_memory);
        size_t region_size = PAGE_ALIGN_UP(current->end - current->start);
        for (size_t offset = 0; offset < region_size; offset += PAGE_SIZE)
        {
            vaddr_t va = current->start + offset;
            paddr_t src_phys = mmu_arch_virt_to_phys(&curr, va);
            if (src_phys == 0)
            {
                continue;
            }

#if USE_EAGER_FORK == 1
            if (current->type == VMA_DEVICE || current->type == VMA_SHARED)
            {
                mmu_arch_map(proc->page_dir, va, src_phys, current->flags);
                continue;
            }
            paddr_t dst_phys = pmm_alloc_frame();
            mmu_copy_contents(src_phys, dst_phys);
            mmu_arch_map(proc->page_dir, va, dst_phys, current->flags);
#else // Cow
            mmu_flags_t new_flags = current->flags;
            new_flags.present = 1;
            new_flags.write = 0;
            new_flags.cow = 1;
            if (current->type == VMA_DEVICE || current->type == VMA_SHARED)
            {
                mmu_arch_map(proc->page_dir, va, src_phys, current->flags);
                continue;
            }

            if (current->type == VMA_ANONYMOUS)
            {
                if (current->top == USER_STACK_TOP)
                {
                    new_memory->stack_vma = new_vma;
                }
                if (current->top == USER_HEAP_BASE)
                {
                    new_memory->heap_vma = new_vma;
                }
            }
            mmu_arch_map(proc->page_dir, va, src_phys, new_flags);

            mmu_arch_protect(parent->page_dir, va, new_flags);

            pmm_ref_frame(src_phys);
#endif
        }
        fprintf(VFS_FD_DEBUG, "new_vma = %p [%p..%p] of type %u flags %x\n", new_vma, new_vma->start, new_vma->end, new_vma->type, new_vma->flags);
        fprintf(VFS_FD_DEBUG, "current = %p [%p..%p] of type %u flags %x\n", current, current->start, current->end, current->type, current->flags);
        vma_insert(new_memory, new_vma);
        current = current->next;
    }

    mmu_arch_load_table(&curr);
    log_debug(MODULE, "done cloning");
    return new_memory;
}

vaddr_t vma_find_free(vma_memory_t *mm, vaddr_t hint, size_t len, vaddr_t search_start, vaddr_t search_end)
{
    len = ALIGN_UP(len, PAGE_SIZE);

    vaddr_t candidate = hint ? PAGE_ALIGN_DOWN(hint) : search_start;
    if (candidate < search_start)
    {
        candidate = search_start;
    }

    vma_t *vma = mm->mmap; // assumes list head is lowest-addressed VMA

    // walk forward to the first VMA that could matter (skip ones entirely below candidate)
    while (vma && vma->end <= candidate)
    {
        vma = vma->next;
    }

    while (candidate + len <= search_end)
    {
        if (!vma || candidate + len <= vma->start)
        {
            // gap [candidate, candidate+len) is free
            return candidate;
        }

        // collide with vma — jump candidate past it and keep scanning
        candidate = ALIGN_UP(vma->end, PAGE_SIZE);
        vma = vma->next;
    }

    return 0; // no free range found
}

void vma_init(process_t *proc)
{
    vma_memory_t *vma_mm = kmalloc(sizeof(vma_memory_t));
    memset(vma_mm, 0, sizeof(vma_memory_t));
    proc->vma = vma_mm;
    vma_mm->page_directory = proc->page_dir;

    log_info(MODULE, "mmap stack proc->vma = %p", proc->vma);
    vma_t *stack = do_mmap_eager(proc->vma, USER_STACK_TOP, PAGE_SIZE * 2, VMA_ANONYMOUS, stack_flags);
    log_info(MODULE, "stack @ %p = {%p, %p}", stack, stack->start, stack->end);
    log_info(MODULE, "mmap heap proc->vma = %p", proc->vma);
    vma_t *heap = do_mmap_eager(proc->vma, USER_HEAP_BASE, PAGE_SIZE * 2, VMA_ANONYMOUS, data_flags);

    vma_mm->start_stack = stack->end;
    vma_mm->start_heap = heap->start;
    vma_mm->end_heap = heap->end;
    vma_mm->heap_vma = heap;
    vma_mm->stack_vma = stack;
}

// mmap syscall bottom half
vma_t *do_mmap(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags)
{
    ENTER_FUNC(MODULE, "%p, %p, 0x%x, %u, 0x%x", mm, addr, size, type, flags);
    // page-align
    vaddr_t base = (vaddr_t)(addr & ~(PAGE_SIZE - 1));
    log_debug(MODULE, "base = %p, addr = %p", base, addr);
    log_debug(MODULE, "flags = %x", flags);
    size_t len = ALIGN_UP(size, PAGE_SIZE);

    // allocate and fill the vma
    vma_t *vma = kmalloc(sizeof(vma_t));
    memset(vma, 0, sizeof(vma_t));
    vma->start = base;
    vma->end = base + len;
    vma->top = base;
    vma->flags = flags;
    vma->type = type;
    vma->next = NULL;
    vma->prev = NULL;

    // demand paging

    vma_insert(mm, vma);
    return vma;
}

// munmap — unmaps pages and removes the VMA entry
int do_munmap(vma_memory_t *mm, vaddr_t addr, size_t size)
{
    return 0;
}

vma_t *do_mmap_eager(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags)
{
    ENTER_FUNC(MODULE, "%p, %p, 0x%x, %u, 0x%x", mm, addr, size, type, flags);
    // page-align
    vaddr_t base = (vaddr_t)(addr & ~(PAGE_SIZE - 1));
    log_debug(MODULE, "base = %p, addr = %p", base, addr);
    log_debug(MODULE, "flags = %x", flags);
    size_t len = ALIGN_UP(size, PAGE_SIZE);

    // allocate and fill the vma
    vma_t *vma = kmalloc(sizeof(vma_t));
    memset(vma, 0, sizeof(vma_t));
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
    vma->next = NULL;
    vma->prev = NULL;

    // eagerly map pages (no demand paging yet)
    if (flags.grows_down)
    {
        for (vaddr_t va = base; base - len < va; va -= PAGE_SIZE)
        {
            paddr_t frame = pmm_alloc_frame();
            // virt_addr virt_frame = phys_to_virt_auto(frame);
            // log_debug(MODULE, "phys = %p | virt = %p", frame, virt_frame);
            log_debug(MODULE, "mapping virt %p .. phys %p", va, frame);
            log_debug(MODULE, "mm = %p, mm->page_directory = %p", mm, &mm->page_directory);
            mmu_arch_map(mm->page_directory, va, frame, flags);
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
            log_debug(MODULE, "mapping virt %p .. phys %p", va, frame);
            log_debug(MODULE, "mm = %p, mm->page_directory = %p", mm, &mm->page_directory);
            mmu_arch_map(mm->page_directory, va, frame, flags);
            memset((void *)va, 0, PAGE_SIZE);
        }
    }

    vma_insert(mm, vma);
    return vma;
}
