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
#include <util/binary.h>

#define MAP_FAILED ((void *)-1)

#define MODULE     "MMAP"

vaddr_t sys_mm_mmap(vma_memory_t *mm, vaddr_t addr_hint, size_t length, mmu_flags_t mmu_flags, int flags, fd_t fd, off_t offset, pledge_flags_t pledge)
{
    ENTER_FUNC(MODULE, "%p, %p, %u, 0x%x, 0x%x, %i, %u", mm, addr_hint, length, mmu_flags, flags, fd, offset);

    vaddr_t target;
    if (FLAG_IS_SET(flags, MAP_FIXED))
    {
        return -ENOSYS;
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
        // Both PROT_GROWSUP and PROT_GROWSDOWN were specified in prot.
        return (void*)-EINVAL;
    }

    if (length == 0)
    {
        // (since Linux 2.6.12) length was 0.
        return (void*)-EINVAL;
    }

    vaddr_t start = (vaddr_t)addr;
    if (start & ~PAGE_MASK || length & ~PAGE_MASK || offset & ~PAGE_MASK)
    {
        log_debug(MODULE, "i dont like addr, length, or offset");
        // We don't like addr, length, or offset (e.g., they are too large,
        // or not aligned on a page boundary).
        return (void*)-EINVAL;
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
    vma_t *vma;
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

    return (void*)sys_mm_mmap(proc->vma, start, length, mmu_flags, flags, fd, offset, pledge);
}

SYSCALL_DEFINE6_PLEDGE(memory_mmap, void *, size_t, int, int, fd_t, off_t);

int memory_munmap(void *addr, size_t length, pledge_flags_t pledge)
{
    return -1;
}

SYSCALL_DEFINE2_PLEDGE(memory_munmap, void *, size_t);
