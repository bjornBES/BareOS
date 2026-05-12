/*
 * File: mman.c
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mman.h"
#include "syscall.h"

void *mmap(void *addr, size_t length, int prot, int flags, fd_t fd, off_t offset)
{
    return (void *)syscall6(SYS_MMAP, (uintptr_t)addr, (uintptr_t)length, (uintptr_t)prot, (uintptr_t)flags, (uintptr_t)fd, (uintptr_t)offset);
}