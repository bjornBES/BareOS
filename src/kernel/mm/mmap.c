/*
 * File: mmap.c
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/mmap.h"
#include "mm/vmm/vmm.h"
#include "syscall/syscall.h"
#include "syscall/pledge.h"
#include "mm/memdefs.h"
#include "mm/mmu/mmu.h"
#include "mm/pmm.h"
#include "debug/debug.h"
#include "task/process.h"
#include "errno/errno.h"
#include "kernel.h"
#include "math.h"
#include <util/binary.h>

#define MAP_FAILED ((void *)-1)

#define MODULE     "MMAP"

vaddr_t sys_mm_mmap(vma_memory_t *mm, vaddr_t addr_hint, size_t length, mmu_flags_t mmu_flags, int flags, fd_t fd, off_t offset, pledge_flags_t pledge)
{
    ENTER_FUNC(MODULE, "%p, %p, %u, 0x%x, 0x%x, %i, %u", mm, addr_hint, length, mmu_flags, flags, fd, offset);

    vaddr_t target;
    if (FLAG_IS_SET(flags, MAP_FIXED))
    {
        target = addr_hint;
        vma_t *vma = vma_find(mm, target);

        vaddr_t start = target & PAGE_MASK;
        vaddr_t end = PAGE_ALIGN_UP(target + length);
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

            for (vaddr_t va = start; va < chunk_end; va += PAGE_SIZE)
            {
                if (mmu_arch_is_present(mm->page_directory, va))
                {
                    paddr_t frame = mmu_arch_unmap(mm->page_directory, va);
                    if (frame == 0)
                    {
                        continue;
                    }
                    pmm_deref_frame(frame);
                }
            }

            vma->flags = mmu_flags;
            vma->type = VMA_ANONYMOUS;
            vma->flags.user = 1;
            vma->flags.present = 0;

            start = chunk_end;
            if (start < end)
            {
                vma = vma->next; // assumes sorted-list adjacency, no gap
            }
        }

        // vma now covers exactly [target, target+len) — same bookkeeping win as mprotect
        return target;
    }
    else
    {
        target = vma_find_free(mm, addr_hint, length, MMAP_ARENA_START, MMAP_ARENA_END);
        log_debug(MODULE, "found %p", target);
        if (target == 0)
        {
            return -ENOMEM;
        }
    }

    vma_t *vma = NULL;
    if (FLAG_IS_SET(flags, MAP_ANONYMOUS))
    {
        vma = do_mmap(mm, target, length, VMA_ANONYMOUS, mmu_flags);
    }

    if (vma == NULL)
    {
        return -ENOMEM;
    }

    log_debug(MODULE, "mmap result = %p", vma->start);
    return vma->start;
}

void *memory_mmap(void *addr, size_t length, int prot, int flags, fd_t fd, off_t offset, pledge_flags_t pledge)
{
    ENTER_FUNC(MODULE, "%p, %u, %x, %x, %i, %u", addr, length, prot, flags, fd, offset);
    int grows = FLAG_GET(prot, PROT_GROWSDOWN | PROT_GROWSUP);
    if (grows == (PROT_GROWSDOWN | PROT_GROWSUP))
    {
        log_err(MODULE, "Both PROT_GROWSUP and PROT_GROWSDOWN were specified in prot");
        // Both PROT_GROWSUP and PROT_GROWSDOWN were specified in prot.
        return (void *)-EINVAL;
    }

    if (length == 0)
    {
        log_err(MODULE, "length was 0");
        // (since Linux 2.6.12) length was 0.
        return (void *)-EINVAL;
    }

    vaddr_t start = (vaddr_t)addr;
    if (start & ~PAGE_MASK || length & ~PAGE_MASK || offset & ~PAGE_MASK)
    {
        log_err(MODULE, "i dont like addr, length, or offset");
        // We don't like addr, length, or offset (e.g., they are too large,
        // or not aligned on a page boundary).
        return (void *)-EINVAL;
    }

    {
        // TODO: ENFILE
        // The system-wide limit on the total number of open files has
        // been reached.
    }

    {
        // TODO: ENOMEM
        // We don't like addr, because it exceeds the virtual address
        // space of the CPU.
    }

    process_t *proc = process_get_current();
    vma_t *vma = NULL;
    mmu_flags_t mmu_flags = {0};
    if (addr != NULL)
    {
        vma = vma_find(proc->vma, start);
    }
    if (prot != PROT_NONE)
    {
        if (vma != NULL)
        {
            mmu_flags = vma->flags;
        }
        mmu_flags.user = 1;
        mmu_flags.present = FLAG_IS_SET(prot, PROT_READ);
        mmu_flags.write = FLAG_IS_SET(prot, PROT_WRITE);
        mmu_flags.exec = FLAG_IS_SET(prot, PROT_EXEC);
        if (FLAG_IS_SET(prot, PROT_GROWSDOWN) && !FLAG_IS_SET(prot, PROT_GROWSUP))
        {
            mmu_flags.grows_down = 1;
        }
        else
        {
            mmu_flags.grows_down = 0;
        }
    }

    return (void *)sys_mm_mmap(proc->vma, start, length, mmu_flags, flags, fd, offset, pledge);
}

SYSCALL_DEFINE6_PLEDGE(memory_mmap, void *, size_t, int, int, fd_t, off_t);

int memory_munmap(void *addr, size_t length, pledge_flags_t pledge)
{
    return -1;
}

SYSCALL_DEFINE2_PLEDGE(memory_munmap, void *, size_t);
