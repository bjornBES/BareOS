/*
 * File: signal.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "signal.h"
#include "syscall.h"

int sigaction(int signum, sigaction_t *action, sigaction_t *old_action)
{
    return syscall4(SYS_SIGACTION, (uintptr_t)signum, (uintptr_t)action, (uintptr_t)old_action, sizeof(sigset_t));
}

int sigreturn()
{
    return syscall0(SYS_SIGRETURN);
}

int kill(pid_t pid, int sig)
{
    return syscall2(SYS_KILL, (uintptr_t)pid, (uintptr_t)sig);
}
