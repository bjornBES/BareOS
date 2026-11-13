/*
 * File: wait.c
 * File Created: 10 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sys/wait.h"
#include "syscall.h"

pid_t waitpid(pid_t pid, int *wstatus, int options)
{
    return 0; // syscall3(SYS_WAITPID, (uintptr_t)pid, (uintptr_t)wstatus, (uintptr_t)options);
}
