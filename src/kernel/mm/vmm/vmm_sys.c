/*
 * File: vmm_sys.c
 * File Created: 07 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vmm_sys.h"
#include "vmm_types.h"
#include "vmm.h"
#include "mm/pmm.h"
#include "kernel/memory.h"
#include "task/process.h"
#include "errno/errno.h"
#include "math.h"
#include <util/binary.h>

#define MODULE "VMA-SYS"

void *do_sys_brk(vma_memory_t *mm, vaddr_t new_addr)
{
    ENTER_FUNC(MODULE, "%p, %p", mm, new_addr);
    vma_t *heap = mm->heap_vma;

    if (new_addr == 0)
    {
        log_debug(MODULE, "returning %p from brk", heap->top);
        return (void *)heap->top;
    }
    if (new_addr < heap->start)
    {
        log_debug(MODULE, "returning %p from brk", heap->top);
        return (void *)heap->top;
    }

    vaddr_t new_end_aligned = ALIGN_UP(new_addr, PAGE_SIZE);
    if (new_end_aligned > heap->end)
    {
        while (heap->end < new_end_aligned)
        {
            if (vma_grow(mm, heap) != RETURN_GOOD)
            {
                break;
            }
        }
    }
    else if (new_end_aligned < heap->end)
    {
        while (heap->end > new_end_aligned)
        {
            if (vma_shrink(mm, heap) != RETURN_GOOD)
            {
                break;
            }
        }
    }

    heap->top = heap->end;
    log_debug(MODULE, "returning %p from brk", heap->top);
    return (void *)heap->top;
}

void *vma_brk(void *addr)
{
    process_t *proc = process_get_current();
    return do_sys_brk(proc->vma, (vaddr_t)addr);
}

SYSCALL_DEFINE1(vma_brk, void *);

int do_mprotect(vma_memory_t *mm, vma_t *vma, vaddr_t addr, size_t len, mmu_flags_t new_flags)
{
    vaddr_t start = addr & PAGE_MASK;
    vaddr_t end = PAGE_ALIGN_UP(addr + len);

    while (start < end)
    {
        vaddr_t chunk_end = min(end, vma->end);

        if (start > vma->start)
        {
            vma = vma_split(mm, vma, start); // vma now starts exactly at "start"
        }

        if (end < vma->end)
        {
            vma_split(mm, vma, end); // chops the tail off; discard return value
        }

        vma->flags = new_flags;

        for (vaddr_t va = start; va < chunk_end; va += PAGE_SIZE)
        {
            mmu_arch_protect(mm->page_directory, va, new_flags); // update PTE, not remap
        }

        start = chunk_end;
        if (start < end)
        {
            vma = vma->next; // assumes sorted-list adjacency, no gap
        }
    }
    return RETURN_GOOD;
}

int memory_mprotect(void *addr, size_t len, int prot)
{
    mmu_flags_t flags;
    int grows = FLAG_GET(prot, PROT_GROWSDOWN | PROT_GROWSUP);
    if (grows == (PROT_GROWSDOWN | PROT_GROWSUP))
    {
        // Both PROT_GROWSUP and PROT_GROWSDOWN were specified in prot.
        return -EINVAL;
    }

    vaddr_t start = (vaddr_t)addr;
    if (start & ~PAGE_MASK)
    {
        // addr is not a valid pointer,
        // or not a multiple of the system page size.
        return -EINVAL;
    }
    if (len == 0)
    {
        return RETURN_GOOD;
    }
    size_t length = PAGE_ALIGN_UP(len);
    vaddr_t end = start + length;
    if (end <= start)
    {
        // Addresses in the range [addr, addr+size-1] are invalid for
        // the address space of the process, or specify one or more
        // pages that are not mapped.
        return -ENOMEM;
    }

    process_t *proc = process_get_current();
    vma_t *vma = vma_find(proc->vma, start);
    if (!vma)
    {
        // Addresses in the range [addr, addr+size-1] are invalid for
        // the address space of the process, or specify one or more
        // pages that are not mapped.
        return -ENOMEM;
    }

    if (prot != PROT_NONE)
    {
        flags = vma->flags;
        flags.present = FLAG_IS_SET(prot, PROT_READ);
        flags.write = FLAG_IS_SET(prot, PROT_WRITE);
        flags.exec = FLAG_IS_SET(prot, PROT_EXEC);
        flags.grows_down = FLAG_IS_SET(prot, PROT_GROWSDOWN);
        flags.grows_down = !FLAG_IS_SET(prot, PROT_GROWSUP);
    }

    return do_mprotect(proc->vma, vma, start, length, flags);
}

SYSCALL_DEFINE3(memory_mprotect, void *, size_t, int);
