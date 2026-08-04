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
#include "kernel/string.h"
#include "kernel/ivt.h"
#include "task/process.h"
#include "task/signals/exit.h"
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

void signal_kernel_signal_handler_core(thread_t *t, int signum, siginfo_t *info, void *ucontext)
{
    ENTER_FUNC(MODULE, "%p, %u, %p, %p", t, signum, info, ucontext);
    log_warn(MODULE, "TODO core dumped");
    do_exit(128 | signum, t->proc);
    __builtin_unreachable();
}

void signal_kernel_signal_handler_term(thread_t *t, int signum, siginfo_t *info, void *ucontext)
{
    ENTER_FUNC(MODULE, "%p, %u, %p, %p", t, signum, info, ucontext);
    // log_crit(MODULE, "kernel handler of signal %u", signum);
    // log_warn(MODULE, "TODO core dumped");
    do_exit(128 | signum, t->proc);
    __builtin_unreachable();
}

void signal_kernel_signal_handler_ignore(thread_t *t, int signum, siginfo_t *info, void *ucontext)
{
    ENTER_FUNC(MODULE, "%p, %u, %p, %p", t, signum, info, ucontext);
    // log_crit(MODULE, "kernel handler of signal %u", signum);
    return;
}

void signal_kernel_signal_handler_continue(thread_t *t, int signum, siginfo_t *info, void *ucontext)
{
    ENTER_FUNC(MODULE, "%p, %u, %p, %p", t, signum, info, ucontext);
    log_crit(MODULE, "kernel handler of signal %u", signum);
    log_warn(MODULE, "TODO core dumped");
    signal_send_group(t->proc, SIGKILL);
}

void signal_kernel_signal_handler_stop(thread_t *t, int signum, siginfo_t *info, void *ucontext)
{
    ENTER_FUNC(MODULE, "%p, %u, %p, %p", t, signum, info, ucontext);
    log_crit(MODULE, "kernel handler of signal %u", signum);
    log_warn(MODULE, "TODO core dumped");
    signal_send_group(t->proc, SIGKILL);
}

sigaction_t kernel_default[32] = {0};
#define KERNEL_HANDLER_FUNC(n, ident)                                                 \
    memset(&kernel_default[n], 0, sizeof(sigaction_t));                               \
    kernel_default[n].handler.default_handler = signal_kernel_signal_handler_##ident; \
    kernel_default[n].sa_mask = 0;                                                    \
    kernel_default[n].sa_flags = SA_SIGINFO;

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
    // ENTER_FUNC(MODULE, "%p, %p, %p", t, action, info);
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
        sigaction_t handler = proc->signal_table.actions[i];
        if (handler.handler.sa_handler == SIG_DFL)
        {
            return RETURN_FAILED;
        }
        (*action) = &proc->signal_table.actions[i];
        return RETURN_GOOD;
    }

    return RETURN_ERROR;
}

int signal_get_action(thread_t *t, int signum, sigaction_t *out)
{
    // ENTER_FUNC(MODULE, "%p, %u, %p", t, signum, out);
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

void deliver_signal(thread_t *t, syscall_info *arch_info, intr_frame_t *regs, sigaction_t *action, siginfo_t *info, bool default_action)
{
    ENTER_FUNC(MODULE, "%p, %p, %p, %p, %p, %s", t, arch_info, regs, action, info, default_action BOOL_TO_STRING);
    log_debug(MODULE, "delivering signal %u", info->si_signo);
    intr_frame_t sig_frame;
    memcpy(&sig_frame, regs, sizeof(intr_frame_t));
    log_debug(MODULE, "doing setup");
    signal_arch_setup_frame(t, &sig_frame, info, action, default_action);
    if (default_action)
    {
        return;
    }
    ivt_dump_frame(&sig_frame);
    log_debug(MODULE, "running dispatch");
    signal_arch_dispatch(&sig_frame);
}

void signal_try_deliver(thread_t *t, syscall_info *arch_info, intr_frame_t *regs)
{
    // ENTER_FUNC(MODULE, "%p, %p, %p", t, arch_info, regs);
    sigaction_t *h = NULL;
    sigaction_t kill;
    {
        if (signal_get_action(t, SIGKILL, &kill) == RETURN_GOOD)
        {
            log_debug(MODULE, "got kill to %u", t->proc->pid);
            if (kill.handler.sa_sigaction == NULL)
            {
                h = signal_default_action(t, SIGKILL);
                return;
            }
            else
            {
                h = &kill;
            }
        }
    }
    siginfo_t info;
    bool default_action = false;
    int state = 0;
    if (h == NULL)
    {
        state = signal_get(t, &info, &h);

        if (state == RETURN_ERROR)
        {
            return;
        }

        if (h == NULL)
        {
            log_debug(MODULE, "use kernel handler");
            default_action = true;
            h = signal_default_action(t, info.si_signo);
        }
    }

    deliver_signal(t, arch_info, regs, h, &info, default_action);
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

sigaction_t *signal_default_action(thread_t *t, int signum)
{
    return &kernel_default[signum];
}

void signal_init()
{
    KERNEL_HANDLER_FUNC(SIGABRT, core);     // Abort signal from abort(3)
    KERNEL_HANDLER_FUNC(SIGALRM, term);     // Timer signal from alarm(2)
    KERNEL_HANDLER_FUNC(SIGBUS, core);      // Bus error (bad memory access)
    KERNEL_HANDLER_FUNC(SIGCHLD, ignore);   // Child stopped, terminated, or continued
    KERNEL_HANDLER_FUNC(SIGCONT, continue); // Continue if stopped
    KERNEL_HANDLER_FUNC(SIGFPE, core);      // Erroneous arithmetic operation
    KERNEL_HANDLER_FUNC(SIGHUP, term);      // Hangup detected on controlling terminal or death of controlling process
    KERNEL_HANDLER_FUNC(SIGILL, core);      // Illegal Instruction
    KERNEL_HANDLER_FUNC(SIGINT, term);      // Interrupt from keyboard
    KERNEL_HANDLER_FUNC(SIGIO, term);       // I/O now possible
    KERNEL_HANDLER_FUNC(SIGIOT, core);      // IOT trap. A synonym for SIGABRT
    KERNEL_HANDLER_FUNC(SIGKILL, term);     // Kill signal
    KERNEL_HANDLER_FUNC(SIGPIPE, term);     // Broken pipe: write to pipe with no readers; see pipe(7)
    KERNEL_HANDLER_FUNC(SIGPROF, term);     // Profiling timer expired
    KERNEL_HANDLER_FUNC(SIGPWR, term);      // Power failure (System V)
    KERNEL_HANDLER_FUNC(SIGQUIT, core);     // Quit from keyboard
    KERNEL_HANDLER_FUNC(SIGSEGV, core);     // Invalid memory reference
    KERNEL_HANDLER_FUNC(SIGSTKFLT, term);   // Stack fault on coprocessor (unused)
    KERNEL_HANDLER_FUNC(SIGSTOP, stop);     // Stop process
    KERNEL_HANDLER_FUNC(SIGTSTP, stop);     // Stop typed at terminal
    KERNEL_HANDLER_FUNC(SIGSYS, core);      // Bad system call
    KERNEL_HANDLER_FUNC(SIGTERM, term);     // Termination signal
    KERNEL_HANDLER_FUNC(SIGTRAP, core);     // Trace/breakpoint trap
    KERNEL_HANDLER_FUNC(SIGTTIN, stop);     // Terminal input for background process
    KERNEL_HANDLER_FUNC(SIGTTOU, stop);     // Terminal output for background process
    KERNEL_HANDLER_FUNC(SIGURG, ignore);    // Urgent condition on socket
    KERNEL_HANDLER_FUNC(SIGUSR1, term);     // User-defined signal 1
    KERNEL_HANDLER_FUNC(SIGUSR2, term);     // User-defined signal 2
    KERNEL_HANDLER_FUNC(SIGVTALRM, term);   // Virtual alarm clock
    KERNEL_HANDLER_FUNC(SIGXCPU, core);     // CPU time limit exceeded
    KERNEL_HANDLER_FUNC(SIGXFSZ, core);     // File size limit exceeded
    KERNEL_HANDLER_FUNC(SIGWINCH, ignore);  // Window resize signal
}
