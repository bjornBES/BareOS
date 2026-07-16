/*
 * File: prctl.c
 * File Created: 07 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "prctl.h"

#include "kernel/asm/MSR/MSR.h"
#include "kernel/memory.h"
#include "syscall/syscall.h"
#include "errno/errno.h"
#include "task/threading/thread.h"
#include "task/threading/scheduling/scheduler.h"

#define MODULE "PRCTL"

int arch_prctl(int code, uint64_t addr)
{
    thread_t *current_thread = scheduler_get_current();
    switch (code)
    {
        case ARCH_SET_FS :
            current_thread->fs_base = addr;
            wrmsr(MSR_FS_BASE, addr);
            break;
        case ARCH_SET_GS :
            current_thread->gs_base = addr;
            wrmsr(MSR_KERNEL_GS_BASE, addr); // careful: user GS, not your per-CPU GS
            break;
        case ARCH_GET_FS :
            copy_to_user((void *)addr, (void *)&current_thread->fs_base, sizeof(uint64_t));
            break;
        case ARCH_GET_GS :
            copy_to_user((void *)addr, (void *)&current_thread->gs_base, sizeof(uint64_t));
            break;
        default :
            log_err(MODULE, "code = %lx", code);
            PRINT_ERROR(EINVAL);
            return -EINVAL;
    }
    return RETURN_GOOD;
}

SYSCALL_DEFINE2(arch_prctl, int, uint64_t);
