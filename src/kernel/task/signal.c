/*
 * File: signal.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 23:28:40
 * Modified By: BjornBEs
 * -----
 */

#include "signal.h"
#include "signal_type.h"
#include "process.h"
#include "syscall/syscall.h"
#include "memory/memdefs.h"
#include "errno/errno.h"
#include "threading/scheduling/scheduler.h"

#include <defs.h>
#include <stdbool.h>

#define MODULE "SIGNAL"

extern char __signal_trampoline;

extern void build_signal_frame(signal_frame *frame);

void signal_kernel_signal_handler(int signum, syscall_info *arch_info, signal_info *info)
{
    log_crit(MODULE, "kernel handler of signal %u", signum);
    process_kill(process_get_current()->pid, SIGKILL);
}
void signal_kernel_signal_handler_core(int signum, syscall_info *arch_info, signal_info *info)
{
    log_crit(MODULE, "kernel handler of signal %u", signum);
    log_warn(MODULE, "TODO core dumped");
    process_kill(process_get_current()->pid, SIGKILL);
}

kernel_signal_action kernel_default[32] = {0};
#define KERNEL_HANDLER(n) kernel_default[n].handler = signal_kernel_signal_handler; memset(&kernel_default[n].action, 0, sizeof(signal_action));
#define KERNEL_HANDLER_CORE(n) kernel_default[n].handler = signal_kernel_signal_handler_core; memset(&kernel_default[n].action, 0, sizeof(signal_action));


kernel_signal_action *signal_get_default_kernel_handler(int signal_number)
{
    kernel_signal_action *result = &kernel_default[signal_number];
    log_info(MODULE, "found %p with function %p", result, kernel_default[signal_number].handler);
    return result;
}

bool signal_is_pending(process_t *proc, int signum)
{
    return (proc->signal_queue.signal >> signum) & 1;
}
void signal_enqueue(process_t *proc, int signum)
{
    proc->signal_queue.signal |= (1u << signum);
}

void signal_dequeue(process_t *proc, int signal_number, signal_info *info)
{
    if (BIT_GET(proc->signal_queue.signal, signal_number) == 1)
    {
        // clear the bit
        proc->signal_queue.signal &= ~(1u << signal_number);

        // fill in the info
        info->signal_number = signal_number;
        info->proc_id = 0; // bitmask can't store this, see note below
    }
}

void *signal_get(bool *is_kernel_handler, signal_info *info)
{
    process_t *proc = process_get_current();

    // find first pending signal
    for (int i = 0; i < NUMBER_SIGNAL; i++)
    {
        if (!signal_is_pending(proc, i))
            continue;

        signal_dequeue(proc, i, info);

        signal_action *action = &proc->signal_table.actions[i];

        if (action->handler == NULL)
        {
            // no userspace handler — use default kernel action
            log_debug(MODULE, "use kernel handler");
            *is_kernel_handler = true;
            return signal_get_default_kernel_handler(i);
        }

        *is_kernel_handler = false;
        return action;
    }

    return NULL;
}

int sig_action(int signum, signal_action *action)
{
    if (signum < 0 || signum >= NUMBER_SIGNAL)
        return -EINVAL;
    if (signum == SIGKILL || signum == SIGSTOP)
        return -EINVAL; // can't override these

    process_t *proc = process_get_current();

    // copy from userspace
    signal_action *k_action = &proc->signal_table.actions[signum];
    log_debug(MODULE, "action @ %p", k_action);
    memcpy(k_action, action, sizeof(signal_action));

    return 0;
}
SYSCALL_DEFINE2(sig_action, int, signal_action *);

extern void hexdump(void *ptr, int len);
void signal_try_call(syscall_info *arch_info)
{
    bool is_kernel_handler = false;
    signal_info info;
    void *h = signal_get(&is_kernel_handler, &info);

    if (h == NULL)
    {

        return;
    }

    // it is the first field in both so yes
    // we can do this.
    signal_handler func_handler;

    if (is_kernel_handler)
    {
        kernel_signal_action kernel_action = kernel_default[info.signal_number];
        log_debug(MODULE, "kernel_action = %016llx", kernel_action);
        log_debug(MODULE, "func_handler = %p", kernel_default[info.signal_number].handler);

        kernel_action.handler(info.signal_number, arch_info, &info);
        return;
    }

    func_handler = ((signal_action *)h)->handler;
    signal_frame frame;
    memcpy(&frame.proc_reg, arch_info->regs, sizeof(registers));
    frame.signal_info = info;

    // arch registers
    build_signal_frame(&frame);

    reg_t user_stack = arch_info->regs->sp;
    user_stack -= sizeof(signal_frame);
    user_stack &= ~0xFULL; // 16-byte align — ABI requirement
    log_debug(MODULE, "calling user_stack=%p", user_stack);

    memcpy((void *)user_stack, &frame, sizeof(signal_frame));

    user_stack -= sizeof(uint64_t);
    uint8_t *trampoline_addr = (uint8_t *)USER_VDSO_VIRT;
    hexdump((void *)trampoline_addr, 16);
    *(reg_t *)user_stack = (uint64_t)trampoline_addr; // your trampoline address
    hexdump((void *)user_stack, 16);
    log_debug(MODULE, "calling user_stack=%p = %x", user_stack, *trampoline_addr);

    inline_asm(
        "mov ax, 0x23\n\t"
        "mov ds, ax\n\t"
        "mov es, ax\n\t"
        "push 0x23\n\t"
        "push %[proc_stack]\n\t"
        "pushf\n\t"
#ifdef __x86_64__
        "pop rax\n\t"
        "or rax, 0x200\n\t"
        "and rax, ~0x100\n\t"
        "push rax\n\t"
#else
        "pop edi\n\t"
        "or edi, 0x200\n\t"
        "and edi, ~0x100\n\t"
        "push edi\n\t"
#endif
        "push 0x1B\n\t"
        "push %[proc_entry]\n\t"
#ifdef __x86_64__
        "iretq\n\t"
#else
        "iret\n\t"
#endif
        : :
        [proc_stack] "m"(user_stack),
        [proc_entry] "m"(func_handler),
        [arg1] "D"(info.signal_number));
}

int signal_kill(pid_t proc_id, int sig)
{
    return process_kill(proc_id, sig);
}
SYSCALL_DEFINE2(signal_kill, pid_t, int);

int signal_return(syscall_info *info)
{
    registers *regs = info->regs;
    signal_frame *frame = (signal_frame *)regs->sp;
    memcpy(regs, &frame->proc_reg, sizeof(registers));
    return 0;
}
SYSCALL_DEFINE0_REG(signal_return);

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
