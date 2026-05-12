/*
 * File: vmm.c
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 23:16:19
 * Modified By: BjornBEs
 * -----
 */

#include "vmm.h"
#include "task/assert/assert.h"
#include "memory/memdefs.h"
#include "memory/pmm/pmm.h"
#include "memory/paging/paging_type.h"
#include "memory/paging/paging.h"
#include "memory/allocator/memory_allocator.h"

#define MODULE "VMA"

// find the VMA containing addr — the #PF handler's first call
vma_t *vma_find(vma_memory_t *mm, virt_addr addr)
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

void vma_grow_down(vma_memory_t *mm, vma_t *vma)
{
    virt_addr new_page = vma->start - PAGE_SIZE;

    // make sure we don't collide with the vma below us
    // (the one with the highest start that is still < our start)
    if (vma->prev && vma->prev->end > new_page)
    {
        // stack overflow — no room to grow
        KernelPanic(MODULE, "stack overflow");
    }

    phys_addr frame = pmm_alloc_frame();
    paging_map_page(mm->page_directory, new_page, frame, vma->flags);
    memset(new_page, 0, PAGE_SIZE);

    vma->start = new_page;
    // no re-insert needed — start moved down but relative order in the
    // list hasn't changed (prev->end is still <= our new start)
}

void vma_grow_up(vma_memory_t *mm, vma_t *vma)
{
    virt_addr new_page = vma->end;

    if (vma->next && vma->next->start < new_page + PAGE_SIZE)
    {
        KernelPanic(MODULE, "heap exhausted");
    }

    phys_addr frame = pmm_alloc_frame();
    paging_map_page(mm->page_directory, new_page, frame, vma->flags);
    memset(new_page, 0, PAGE_SIZE);

    vma->end += PAGE_SIZE;
}

void vma_free(process_t *proc)
{
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
        cur = cur->next;

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
            cur->next->prev = new;
        cur->next = new;
    }

    mm->map_count++;
}

vma_memory_t *vma_clone(vma_memory_t *src, process_t *proc)
{
    paging_page_t curr;
    paging_get_current_cr3(&curr);
    paging_load_cr3(kernel_page);

    vma_memory_t *new_memory = kmalloc(sizeof(vma_memory_t));
    memset(new_memory, 0, sizeof(vma_memory_t));
    new_memory->start_stack = src->start_stack;
    new_memory->start_heap  = src->start_heap;
    new_memory->end_heap    = src->end_heap;

    vma_t *current = src->mmap;
    while (current)
    {
        vma_t *new_vma = kmalloc(sizeof(vma_t));
        #if USE_EAGER_FORK == 1
        memcpy(new_vma, current, sizeof(vma_t));
        fprintf(VFS_FD_DEBUG, "new_vma = %p, new_memory = %p", new_vma, new_memory);
        size_t region_size = ALIGN_2_UP((uint64_t)current->end - (uint64_t)current->start, PAGE_SIZE);
        for (size_t offset = 0; offset < region_size; offset += PAGE_SIZE)
        {
            virt_addr va = (virt_addr)((uint64_t)current->start + (uint64_t)offset);
            phys_addr src_phys = paging_get_physical(curr, va);
            if (src_phys == NULL)
            {
                continue;
            }
            
            if (current->type == VMA_DEVICE || current->type == VMA_SHARED)
            {
                paging_map_page(proc->page_dir, va, src_phys, current->flags);
                continue;
            }
            
            phys_addr dst_phys = pmm_alloc_frame();
            page_copy(src_phys, dst_phys);
            paging_map_page(proc->page_dir, va, dst_phys, current->flags);
        }
        fprintf(VFS_FD_DEBUG, "new_vma = %p new_memory = %p", new_vma, new_memory);
        vma_insert(new_memory, new_vma);
        #else // Cow
        #error CoW not implemented yet
        #endif
        current = current->next;
    }

    paging_load_cr3(curr);
    log_debug(MODULE, "done cloning");
    return new_memory;
}

void vma_init(process_t *proc)
{
    vma_memory_t *vma_mm = kmalloc(sizeof(vma_memory_t));
    memset(vma_mm, 0, sizeof(vma_memory_t));
    proc->vma = vma_mm;
    vma_mm->page_directory = proc->page_dir;
}

// mmap syscall bottom half
vma_t *do_mmap(vma_memory_t *mm, virt_addr addr, size_t size, vma_type_t type, memory_flags_t flags)
{
    // page-align
    virt_addr base = (virt_addr)((uint32_64)addr & ~(PAGE_SIZE - 1));
    log_debug(MODULE, "base = %p, addr = %p", base, addr);
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

    // eagerly map pages (no demand paging yet)
    for (virt_addr va = base; va < base + len; va += PAGE_SIZE)
    {
        phys_addr frame = pmm_alloc_frame();
        // virt_addr virt_frame = phys_to_virt_auto(frame);
        // log_debug(MODULE, "phys = %p | virt = %p", frame, virt_frame);
        log_debug(MODULE, "mapping virt %p .. phys %p", va, frame);
        log_debug(MODULE, "mm = %p, mm->page_directory = %p", mm, &mm->page_directory);
        paging_map_page(mm->page_directory, va, frame, flags);
        memset(va, 0, PAGE_SIZE);
    }

    vma_insert(mm, vma);
    return vma;
}

// munmap — unmaps pages and removes the VMA entry
int do_munmap(vma_memory_t *mm, virt_addr addr, size_t size)
{
    return 0;
}
