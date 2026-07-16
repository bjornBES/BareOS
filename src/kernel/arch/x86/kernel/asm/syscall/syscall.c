/*
 * File: syscall.c
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/syscall.h"
#include "syscall.h"

#include "kernel/asm/exceptions/exception.h"
#include "kernel/segments.h"
#include "kernel/asm/cpuid/cpuid.h"
#include "kernel/asm/MSR/MSR.h"
#include "kernel/ivt.h"
#include "kernel/x86.h"
#include "kernel/memory.h"
#include "task/process.h"

#include "syscall/syscall.h"
#include "errno/errno.h"

#define MODULE "X86-SYSCALL"

int syscall_arch_handler(intr_frame_t *regs)
{
    syscall_function_info func_info;
    if (syscall_get_handler(regs->ax, &func_info) != RETURN_GOOD)
    {
        log_crit(MODULE, "Unhandled System call %d", regs->ax);
        ivt_dump_frame(regs);
        if (regs->ax < 255)
        {
            KernelPanic(MODULE, "");
        }
        regs->ax = -ENOSYS;
        return RETURN_FAILED;
    }
    process_t *proc = process_get_current();
    syscall_info info;
    if (func_info.abi == SYSV64)
    {
        info.arg1 = regs->di;
        info.arg2 = regs->si;
        info.arg3 = regs->dx;
        info.arg4 = regs->r10;
        info.arg5 = regs->r8;
        info.arg6 = regs->r9;
        info.sys_number = regs->ax;
    }

    fprintf(VFS_FD_DEBUG, "System call %d from pid %u\n", info.sys_number, proc->pid);
    
    // ivt_dump_frame(regs);

    info.regs = regs;
    uintptr_t ret = syscall_handler_func(&info, &func_info, regs);
    regs->ax = ret;
    if (ret == -ENOSYS)
    {
        log_crit(MODULE, "Unhandled System call %d", info.sys_number);
        log_crit(MODULE, "  arg1=%x  arg2=%x  arg3=%x  arg4=%x  arg5=%x  arg6=%x", info.arg1, info.arg2, info.arg3, info.arg4, info.arg5, info.arg6);
        return RETURN_FAILED;
    }
    else
    {
    }
    return RETURN_GOOD;
}

void syscall_dispatch(syscall_frame_t *regs)
{
    intr_frame_t frame;
    frame.r15 = regs->r15;
    frame.r14 = regs->r14;
    frame.r13 = regs->r13;
    frame.r12 = regs->r12;
    frame.r11 = regs->r11;
    frame.r10 = regs->r10;
    frame.r9 = regs->r9;
    frame.r8 = regs->r8;
    frame.di = regs->di;
    frame.si = regs->si;
    frame.bp = regs->bp;
    frame.bx = regs->bx;
    frame.dx = regs->dx;
    frame.cx = regs->cx;
    frame.ax = regs->ax;
    frame.sp = regs->sp;
    frame.pc = regs->pc;
    frame.flags = regs->flags;
    // fprintf(VFS_FD_DEBUG, "\t{ frame @ %p }\n", regs);
    // fprintf(VFS_FD_DEBUG, "\t{ ax = 0x%lx, bx = 0x%lx, cx = 0x%lx, dx = 0x%lx }\n", regs->ax, regs->bx, regs->cx, regs->dx);
    // fprintf(VFS_FD_DEBUG, "\t{ di = 0x%lx, si = 0x%lx, r8 = 0x%lx, r9 = 0x%lx }\n", regs->di, regs->si, regs->r8, regs->r9);
    // fprintf(VFS_FD_DEBUG, "\t{ r10 = 0x%lx, r11 = 0x%lx, r12 = 0x%lx, r13 = 0x%lx }\n", regs->r10, regs->r11, regs->r12, regs->r13);
    // fprintf(VFS_FD_DEBUG, "\t{ r14 = 0x%lx, r15 = 0x%lx, bp = 0x%lx, flags = 0x%lx }\n", regs->r14, regs->r15, regs->bp, regs->flags);
    // fprintf(VFS_FD_DEBUG, "\t{ pc = %p, sp = %p }\n", regs->pc, regs->sp);
    syscall_arch_handler(&frame);
    memcpy(&regs->r15, &frame.r15, 15 * sizeof(reg_t));
    regs->sp = frame.sp;
    regs->pc = frame.pc;
    regs->flags = frame.flags;
}

void arch_syscall_init()
{
    ivt_arch_set_handler(EXC_SYSCALL, syscall_arch_handler);
    syscall_per_cpu_init();
}

extern void syscall_entry();

void syscall_per_cpu_init()
{
#ifdef __i686__
    return;
#else
    cpuid_regs_t regs;
    cpuid(0x80000001, 0, &regs);
    if (BIT_GET(regs.edx, 11) && msr_has_msr() == RETURN_GOOD)
    {
        uint64_t efer = msr_get_64(MSR_EFER);
        msr_set_64(MSR_EFER, efer | MSR_EFER_SCE);

        vaddr_t entry_syscall = (vaddr_t)syscall_entry;
        uint64_t star = 0;
        star |= (uint64_t)KERNEL_CODE_SELECTOR << 32;
        star |= (uint64_t)(UNUSED_SELECTOR | 3) << 48;
        log_debug(MODULE, "star = %lx", star);
        msr_set_64(MSR_STAR, star);
        msr_set_64(MSR_LSTAR, entry_syscall);
        msr_set_64(MSR_SYSCALL_MASK, X86_EFLAGS_AC | X86_EFLAGS_DF | X86_EFLAGS_IF | X86_EFLAGS_TF);
    }
#endif
}
