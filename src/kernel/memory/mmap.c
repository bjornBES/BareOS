/*
 * File: mmap.c
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mmap.h"
#include "syscall/syscall.h"
#include "syscall/pledge.h"
#include "memory/paging/paging.h"
#include "memory/pmm/pmm.h"
#include "debug/debug.h"
#include "task/process.h"
#include "kernel.h"

#define MAP_FAILED (void *)-1

#define MODULE "MMAP"

void *memory_mmap(void *addr, size_t length, int prot, int flags, fd_t fd, off_t offset, pledge_flags_t pledge)
{
    log_debug(MODULE, "memory_mmap(%p, %u, %x, %x, %i, %u)", addr, length, prot, flags, fd, offset);
    if ((flags & MAP_ANONYMOUS) == 0 || fd >= VFS_FS_NEXT || offset != 0)
    {
        FUNC_NOT_IMPLEMENTED(MODULE, "memory_mmap");
        return MAP_FAILED;
    }

    if (addr != NULL)
    {
        FUNC_NOT_IMPLEMENTED(MODULE, "memory_mmap");
        return MAP_FAILED;
    }

    if (flags & MAP_ANONYMOUS)
    {
        if ((flags & MAP_PRIVATE) == 0)
        {
            log_err(MODULE, "error");
            return MAP_FAILED;
        }
        size_t alloc_size = 0;
        // TODO:
        /* virt_addr result = current_process->heap_top;
        while (alloc_size < length)
        {
            phys_addr frame = paging_alloc_frame();
            paging_map_page(current_process->page_dir, current_process->heap_top, frame, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
            current_process->heap_top += USER_HEAP_INITIAL;
            alloc_size += USER_HEAP_INITIAL;
        } */
        return NULL;
    }

    return MAP_FAILED;
}

SYSCALL_DEFINE6_PLEDGE(memory_mmap, void *, size_t, int, int, fd_t, off_t);

int memory_munmap(void *addr, size_t length, pledge_flags_t pledge)
{
    return -1;
}

SYSCALL_DEFINE2_PLEDGE(memory_munmap, void *, size_t);
