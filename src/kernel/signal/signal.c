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
    memset(&kernel_default[n].action, 0, sizeof(sigaction_t));
#define KERNEL_HANDLER_CORE(n)                                     \
    kernel_default[n].handler = signal_kernel_signal_handler_core; \
    memset(&kernel_default[n].action, 0, sizeof(sigaction_t));

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
    BIT_SET(t->blocked_signals, signum);
}

void signal_unmask(thread_t *t, int signum)
{
    BIT_UNSET(t->blocked_signals, signum);
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
    if (t->blocked_signals)
    {
        return t->blocked_signals & signum;
    }
    return false;
}

void signal_dequeue(process_t *proc, int signal_number, siginfo_t *info)
{
    if (BIT_GET(proc->signal_queue.signal, signal_number) == 1)
    {
        // clear the bit
        proc->signal_queue.signal &= ~(1u << signal_number);

        // fill in the info
        info->si_signo = signal_number;
        info->si_common.si_pid = proc->pid;
    }
}

int signal_get(thread_t *t, siginfo_t *info, sigaction_t **action)
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

int signal_get_action(thread_t *t, int signum, sigaction_t *out)
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

int signal_set_action(thread_t *t, int signum, sigaction_t *action)
{
    // copy from userspace
    sigaction_t *k_action = &t->proc->signal_table.actions[signum];
    log_debug(MODULE, "action @ %p", k_action);

    int state = copy_from_user(k_action, action, sizeof(sigaction_t));
    return state;
}

extern void hexdump(void *ptr, int len);

void deliver_signal(thread_t *t, syscall_info *arch_info, intr_frame_t *regs, sigaction_t *action, siginfo_t *info)
{
    intr_frame_t sig_frame;
    memcpy(&sig_frame, regs, sizeof(intr_frame_t));
    log_debug(MODULE, "doing setup");
    signal_arch_setup_frame(t, &sig_frame, info, action);
    ivt_dump_frame(&sig_frame);
    log_debug(MODULE, "running dispatch");
    signal_arch_dispatch(&sig_frame);
}

void signal_try_deliver(thread_t *t, syscall_info *arch_info, intr_frame_t *regs)
{
    {
        sigaction_t kill;
        if (signal_get_action(t, SIGKILL, &kill) == RETURN_GOOD)
        {
            log_debug(MODULE, "got kill to %u", t->proc->pid);
            if (kill.handler.sa_sigaction == NULL)
            {
                signal_default_action(t, SIGKILL);
                return;
            }
        }
    }
    siginfo_t info;
    sigaction_t *h = NULL;
    int state = signal_get(t, &info, &h);

    if (state == RETURN_FAILED)
    {
        return;
    }

    if (h == NULL)
    {
        log_debug(MODULE, "use kernel handler");
        signal_default_action(t, info.si_signo);
        return;
    }

    deliver_signal(t, arch_info, regs, h, &info);
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
