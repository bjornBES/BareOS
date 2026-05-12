/*
 * File: syscall.c
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "syscall.h"

#include "exceptions/exception.h"
#include "task/process.h"

#include "syscall/syscall.h"

void syscall_arch_handler(registers *regs, kernel_handler kernel_hand)
{
    process_t *proc = process_get_current();
    syscall_info info;
    if (proc->abi == API_SYSV64)
    {
        info.arg1 = regs->di;
        info.arg2 = regs->si;
        info.arg3 = regs->dx;
        info.arg4 = regs->r10;
        info.arg5 = regs->r8;
        info.arg6 = regs->r9;
        info.sys_number = regs->ax;
    }
    info.regs = regs;
    kernel_hand(&info);
}

void arch_syscall_init()
{
    exception_register_arch_handler(EXC_SYSCALL, syscall_arch_handler);
}