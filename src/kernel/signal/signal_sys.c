/*
 * File: signal_sys.c
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "signal/signal_sys.h"
#include "syscall/syscall.h"
#include "errno/errno.h"

#include "mm/vmm/vmm.h"

#include "task/threading/scheduling/scheduler.h"
#include "task/threading/thread_type.h"
#include "kernel/memory.h"

#define MODULE "signal-sys"

int sig_procmask(int how, sigset_t *set, sigset_t *oldset, size_t sigsetsize)
{
    if (sigsetsize != sizeof(sigset_t))
    {
        SET_ERROR(EINVAL);
        return -EINVAL;
    }

    thread_t *current_thread = scheduler_get_current();
    vma_memory_t *mm = current_thread->proc->vma;

    if (oldset != NULL)
    {
        if (vma_inside(mm, (vaddr_t)oldset) != RETURN_GOOD)
        {
            SET_ERROR(EFAULT);
            return -EFAULT;
        }
        copy_to_user(oldset, &current_thread->signal_mask, sigsetsize);
    }

    if (set != NULL)
    {
        if (vma_inside(mm, (vaddr_t)set) != RETURN_GOOD)
        {
            SET_ERROR(EFAULT);
            return -EFAULT;
        }
        sigset_t new_mask;
        copy_from_user(&new_mask, set, sigsetsize);
        switch (how)
        {
            case SIG_BLOCK :
                current_thread->signal_mask |= new_mask;
                break;
            case SIG_UNBLOCK :
                current_thread->signal_mask &= ~new_mask;
                break;
            case SIG_SETMASK :
                current_thread->signal_mask = new_mask;
                break;
            default :
                SET_ERROR(EINVAL);
                return -EINVAL;
        }
    }

    return RETURN_GOOD;
}

SYSCALL_DEFINE4(sig_procmask, int, sigset_t *, sigset_t *, size_t);
