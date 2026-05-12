/*
 * File: unistd.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "unistd.h"

#include "syscall.h"

size_t read(fd_t fd, void *buf, size_t len)
{
    return syscall3(SYS_READ, (uintptr_t)fd, (uintptr_t)buf, (uintptr_t)len);
}

size_t write(fd_t fd, const void *buf, size_t len)
{
    return syscall3(SYS_WRITE, (uintptr_t)fd, (uintptr_t)buf, (uintptr_t)len);
}

pid_t getpid()
{
    return syscall0(SYS_GETPID);
}

pid_t fork()
{
    return syscall0(SYS_FORK);
}

void _exit(int status)
{
    syscall1(SYS_EXIT, (uintptr_t)status);
    __builtin_unreachable();
}

int execve(const char *filename, const char *argv[], const char *envp[])
{
    return syscall3(SYS_EXECVE, (uintptr_t)filename, (uintptr_t)argv, (uintptr_t)envp);
}

void pledge(uint32_t pledges)
{
    syscall1(SYS_PLEDGE, (uintptr_t)pledges);
}
