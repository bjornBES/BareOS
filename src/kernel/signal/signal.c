/*
 * File: signal.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "signal/signal.h"
#include "signal/signal_type.h"
#include "kernel/signal/signal.h"
#include "kernel/memory.h"
#include "kernel/ivt.h"
#include "task/process.h"
#include "syscall/syscall.h"
#include "mm/memdefs.h"
#include "errno/errno.h"
#include "task/threading/scheduling/scheduler.h"

#include <util/binary.h>
#include <defs.h>
#include <stdbool.h>

#define MODULE "SIGNAL"

extern char __signal_trampoline;

extern void build_signal_frame(signal_arch_frame_t *frame);

void signal_kernel_signal_handler(thread_t *t, int signum)
{
    log_crit(MODULE, "kernel handler of signal %u", signum);
    signal_send(t, SIGKILL);
}

void signal_kernel_signal_handler_core(thread_t *t, int signum)
{
    log_crit(MODULE, "kernel handler of signal %u", signum);
    log_warn(MODULE, "TODO core dumped");
    signal_send(t, SIGKILL);
}

kernel_signal_action kernel_default[32] = {0};
#define KERNEL_HANDLER(n)                                          \
    kernel_default[n].handler = signal_kernel_signal_handler;      \
    memset(&kernel_default[n].action, 0, sizeof(signal_action_t));
#define KERNEL_HANDLER_CORE(n)                                     \
    kernel_default[n].handler = signal_kernel_signal_handler_core; \
    memset(&kernel_default[n].action, 0, sizeof(signal_action_t));

int signal_send(thread_t *t, int signum)
{
    BIT_SET(t->signal_queue.signal, signum);
    return RETURN_GOOD;
}

int signal_send_group(process_t *proc, int signum)
{
    BIT_SET(proc->signal_queue.signal, signum);
    return RETURN_GOOD;
}

void signal_mask(thread_t *t, int signum)
{
    BIT_SET(t->signal_mask, signum);
}

void signal_unmask(thread_t *t, int signum)
{
    BIT_UNSET(t->signal_mask, signum);
}

bool signal_is_pending(thread_t *t, int signum)
{
    if (t->signal_queue.signal)
    {
        return (t->signal_queue.signal >> signum) & 1;
    }
    return (t->proc->signal_queue.signal >> signum) & 1;
}

bool signal_is_masked(thread_t *t, int signum)
{
    if (t->signal_mask)
    {
        return t->signal_mask & signum;
    }
    return false;
}

void signal_dequeue(process_t *proc, int signal_number, signal_info *info)
{
    if (BIT_GET(proc->signal_queue.signal, signal_number) == 1)
    {
        // clear the bit
        proc->signal_queue.signal &= ~(1u << signal_number);

        // fill in the info
        info->signal_number = signal_number;
        info->proc_id = proc->pid;
    }
}

int signal_get(thread_t *t, signal_info *info, signal_action_t **action)
{
    process_t *proc = t->proc;

    // find first pending signal
    for (int i = 0; i < NUMBER_SIGNAL; i++)
    {
        if (!signal_is_pending(t, i))
        {
            continue;
        }
        if (signal_is_masked(t, i))
        {
            continue;
        }

        signal_dequeue(proc, i, info);
        (*action) = &proc->signal_table.actions[i];
        if ((*action) == NULL)
        {
            return RETURN_ERROR;
        }
        return RETURN_GOOD;
    }

    return RETURN_FAILED;
}

int signal_get_action(thread_t *t, int signum, signal_action_t *out)
{
    if (!signal_is_pending(t, signum))
    {
        return RETURN_FAILED;
    }
    if (signal_is_masked(t, signum))
    {
        return RETURN_FAILED;
    }
    out = &t->proc->signal_table.actions[signum];
    return RETURN_GOOD;
}

int signal_set_action(thread_t *t, int signum, signal_action_t *action)
{
    // copy from userspace
    signal_action_t *k_action = &t->proc->signal_table.actions[signum];
    log_debug(MODULE, "action @ %p", k_action);
    memcpy(k_action, action, sizeof(signal_action_t));
    return RETURN_GOOD;
}

int sig_action(int signum, signal_action_t *action)
{
    if (signum < 0 || signum >= NUMBER_SIGNAL)
    {
        return -EINVAL;
    }
    if (signum == SIGKILL || signum == SIGSTOP)
    {
        return -EINVAL; // can't override these
    }

    thread_t *t = scheduler_get_current();

    return signal_set_action(t, signum, action);
}

SYSCALL_DEFINE2(sig_action, int, signal_action_t *);


extern void hexdump(void *ptr, int len);

void signal_try_deliver(thread_t *t, syscall_info *arch_info, intr_frame_t *regs)
{
    {
        signal_action_t kill;
        if (signal_get_action(t, SIGKILL, &kill) == RETURN_GOOD)
        {
            log_debug(MODULE, "got kill to %u", t->proc->pid);
            if (kill.handler == NULL)
            {
                signal_default_action(t, SIGKILL);
                return;
            }
        }
    }
    signal_info info;
    signal_action_t *h = NULL;
    int state = signal_get(t, &info, &h);

    if (state == RETURN_FAILED)
    {
        return;
    }

    if (h == NULL)
    {
        log_debug(MODULE, "use kernel handler");
        signal_default_action(t, info.signal_number);
        return;
    }
    
    signal_action_t *sig_ac = h;
    intr_frame_t sig_frame;
    memcpy(&sig_frame, regs, sizeof(intr_frame_t));
    log_debug(MODULE, "doing setup");
    signal_arch_setup_frame(t, &sig_frame, &info, sig_ac);
    ivt_dump_frame(&sig_frame);
    log_debug(MODULE, "running dispatch");
    signal_arch_dispatch(&sig_frame);
}

int signal_kill(pid_t proc_id, int sig)
{
    return process_kill(proc_id, sig);
}

SYSCALL_DEFINE2(signal_kill, pid_t, int);

int signal_return(syscall_info *info)
{
    signal_arch_restore_frame(info->regs);
    return RETURN_GOOD;
}

SYSCALL_DEFINE0_REG(signal_return);

void signal_default_action(thread_t *t, int signum)
{
    kernel_signal_action kernel_action = kernel_default[signum];
    kernel_action.handler(t, signum);
}

void signal_init()
{
    KERNEL_HANDLER(SIGHUP);
    KERNEL_HANDLER(SIGINT);
    KERNEL_HANDLER_CORE(SIGQUIT);
    KERNEL_HANDLER_CORE(SIGILL);
    KERNEL_HANDLER_CORE(SIGTRAP);
    KERNEL_HANDLER_CORE(SIGABRT);
    KERNEL_HANDLER_CORE(SIGIOT);
    KERNEL_HANDLER_CORE(SIGBUS);
    KERNEL_HANDLER_CORE(SIGFPE);
    KERNEL_HANDLER(SIGKILL);
    KERNEL_HANDLER(SIGUSR1);
    KERNEL_HANDLER_CORE(SIGSEGV);
    KERNEL_HANDLER(SIGUSR2);
    KERNEL_HANDLER(SIGPIPE);
    KERNEL_HANDLER(SIGALRM);
    KERNEL_HANDLER(SIGTERM);
    KERNEL_HANDLER(SIGSTKFLT);
    KERNEL_HANDLER(SIGCHLD);
    KERNEL_HANDLER(SIGCONT);
    KERNEL_HANDLER(SIGSTOP);
    KERNEL_HANDLER(SIGTSTP);
    KERNEL_HANDLER(SIGTTIN);
    KERNEL_HANDLER(SIGTTOU);
    KERNEL_HANDLER(SIGURG);
    KERNEL_HANDLER_CORE(SIGXCPU);
    KERNEL_HANDLER_CORE(SIGXFSZ);
    KERNEL_HANDLER(SIGVTALRM);
    KERNEL_HANDLER(SIGPROF);
    KERNEL_HANDLER(SIGWINCH);
    KERNEL_HANDLER(SIGIO);
    KERNEL_HANDLER(SIGPWR);
    KERNEL_HANDLER_CORE(SIGSYS);
}
