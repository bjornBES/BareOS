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
#include "signal/signal.h"
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
    
    thread_t *current_thread = sched_get_current();
    ctx_dump(&current_thread->ctx);
    vma_memory_t *mm = current_thread->proc->vma;

    if (oldset != NULL)
    {
        if (vma_inside(mm, (vaddr_t)oldset) != RETURN_GOOD)
        {
            SET_ERROR(EFAULT);
            return -EFAULT;
        }
        copy_to_user(oldset, &current_thread->blocked_signals, sigsetsize);
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
                current_thread->blocked_signals |= new_mask;
                break;
            case SIG_UNBLOCK :
                current_thread->blocked_signals &= ~new_mask;
                break;
            case SIG_SETMASK :
                current_thread->blocked_signals = new_mask;
                break;
            default :
                SET_ERROR(EINVAL);
                return -EINVAL;
        }
    }

    return RETURN_GOOD;
}

SYSCALL_DEFINE4(sig_procmask, int, sigset_t *, sigset_t *, size_t);

int rt_sig_action(int signum, const sigaction_t *action, sigaction_t *old_action, size_t sigsetsize)
{
    if (sigsetsize != sizeof(sigset_t))
    {
        ERRNO_RETURN(EINVAL, "sigsetsize (%u) and %u are not the same size", sigsetsize, sizeof(sigset_t));
    }
    if (signum < 0 || signum >= NUMBER_SIGNAL)
    {
        ERRNO_RETURN(EINVAL, "An invalid signal was specified. This will also be generated if an attempt is made to change the action for SIGKILL or SIGSTOP, which cannot be caught or ignored.", "");
    }
    if (signum == SIGKILL || signum == SIGSTOP)
    {
        ERRNO_RETURN(EINVAL, "An invalid signal was specified. This will also be generated if an attempt is made to change the action for SIGKILL or SIGSTOP, which cannot be caught or ignored.", "");
    }

    thread_t *t = sched_get_current();
    ctx_dump(&t->ctx);
    process_t *proc = process_get_current();

    if (old_action != NULL)
    {
        int state = copy_to_user(old_action, &proc->signal_table.actions[signum], sizeof(sigaction_t));
        if (state != RETURN_GOOD)
        {
            return state;
        }
    }

    if (action != NULL)
    {
        return signal_set_action(t, signum, (sigaction_t *)action);
    }

    return RETURN_GOOD;
}

SYSCALL_DEFINE4(rt_sig_action, int, const sigaction_t *, sigaction_t *, size_t);
