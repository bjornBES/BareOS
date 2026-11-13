/*
 * File: syscall.c
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/syscall_archt.h"
#include "asm/syscall_arch.h"
#include "asm/vectors_arch.h"

#include "x86_arch_data.h"

#include "syscall/syscall.h"

#include "entry/desc/gdt/segments.h"

#include "kernel/cpuid/cpuid.h"
#include "kernel/msr/msr.h"
#include "kernel/cpu/cpu_flags.h"

#include "ivt/ivt.h"

#include "debug/debug.h"

#include "memory.h"

#include <config.h>

#define MODULE "x86-syscall"

typedef struct syscall_frame
{
#ifdef CONFIG_64BIT
    reg_t r15;
    reg_t r14;
    reg_t r13;
    reg_t r12;
    reg_t r11;
    reg_t r10;
    reg_t r9;
    reg_t r8;
#endif
    reg_t di;
    reg_t si;
    reg_t bp;
    reg_t bx;
    reg_t dx;
    reg_t cx;
    reg_t ax;

    reg_t pc;
    reg_t flags;
    reg_t sp;
} PACKED syscall_frame_t;

status_t syscall_arch_handler(intr_frame_t *regs)
{
    syscall_function_info_t *func_info;
    if (syscall_get_handler(regs->ax, &func_info))
    {
        trace_crit(MODULE, "Unhandled System call %d", regs->ax);
        regs->ax = (reg_t)KERRNO_TO_POSIX_ERROR(ENOSYS);
        KERRNO_RETURN(ENOSYS, "Unhandled System call %d", regs->ax);
    }
    syscall_info_t info;
    if (func_info->abi == ABI_SYSV64)
    {
        info.args[0] = regs->di;
        info.args[1] = regs->si;
        info.args[2] = regs->dx;
        info.args[3] = regs->r10;
        info.args[4] = regs->r8;
        info.args[5] = regs->r9;
        info.sys_number = regs->ax;
    }

    // if (regs->ax > 1)
    {
        // trace_info(MODULE, "System call %d", info.sys_number);
    }

    // ivt_dump_frame(regs);

    info.regs = regs;
    intptr_t ret_val;
    status_t ret;
    ret = syscall_handler_func(&info, func_info, &ret_val);
    if (ret == ENOSYS)
    {
        trace_crit(MODULE, "Unhandled System call %d", info.sys_number);
        trace_crit(MODULE, "arg1=%x  arg2=%x  arg3=%x  arg4=%x  arg5=%x  arg6=%x", info.args[0], info.args[1], info.args[2], info.args[3], info.args[4], info.args[5]);
        regs->ax = KERRNO_TO_POSIX_ERROR(ENOSYS);
        KERRNO_RETURN(ENOSYS, "Unhandled System call", 0);
    }
    if (func_info->call_dose_return)
    {
        regs->ax = ret_val;
        if (ret)
        {
            regs->ax = KERRNO_TO_POSIX_ERROR(ret);
        }
    }
    else
    {
        // log_debug(MODULE, "done with %u", func_info.number);
    }
    // log_debug(MODULE, "DONE");
    return ret;
}

void syscall_dispatch(syscall_frame_t *regs)
{
    intr_frame_t frame;
    memset(&frame, 0, sizeof(intr_frame_t));
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
    // if (regs->ax > 1)
    // {
    //     ivt_dump_frame(&frame);
    // }
    reg_t syscall = frame.ax;
    syscall_arch_handler(&frame);
    regs->r15 = frame.r15;
    regs->r14 = frame.r14;
    regs->r13 = frame.r13;
    regs->r12 = frame.r12;
    regs->r11 = frame.r11;
    regs->r10 = frame.r10;
    regs->r9 = frame.r9;
    regs->r8 = frame.r8;
    regs->di = frame.di;
    regs->si = frame.si;
    regs->bp = frame.bp;
    regs->bx = frame.bx;
    regs->dx = frame.dx;
    regs->cx = frame.cx;
    regs->ax = frame.ax;
    regs->sp = frame.sp;
    regs->pc = frame.pc;
    regs->flags = frame.flags;
    // if (syscall > 1)
    // {
    //     log_debug(MODULE, "\t{ frame @ %p }", regs);
    //     log_debug(MODULE, "\t{ ax = 0x%lx, bx = 0x%lx, cx = 0x%lx, dx = 0x%lx }", regs->ax, regs->bx, regs->cx, regs->dx);
    //     log_debug(MODULE, "\t{ di = 0x%lx, si = 0x%lx, r8 = 0x%lx, r9 = 0x%lx }", regs->di, regs->si, regs->r8, regs->r9);
    //     log_debug(MODULE, "\t{ r10 = 0x%lx, r11 = 0x%lx, r12 = 0x%lx, r13 = 0x%lx }", regs->r10, regs->r11, regs->r12, regs->r13);
    //     log_debug(MODULE, "\t{ r14 = 0x%lx, r15 = 0x%lx, bp = 0x%lx, flags = 0x%lx }", regs->r14, regs->r15, regs->bp, regs->flags);
    //     log_debug(MODULE, "\t{ pc = %p, sp = %p }", regs->pc, regs->sp);
    //     ivt_dump_frame(&frame);
    // }
}

extern void x86_syscall_entry();

void syscall_per_cpu_init(boot_params_t *bp)
{
    x86_arch_data_t *data = bp->arch_runtime_data;
#ifdef CONFIG_64BIT
trace_debug(MODULE, "data->syscall.has_syscall = %u, data->has_msr = %u", data->syscall.has_syscall, data->has_msr);
    if (data->syscall.has_syscall && data->has_msr)
    {
        uint64_t efer = msr_get_64(MSR_EFER);
        msr_set_64(MSR_EFER, efer | BIT(MSR_EFER_SCE_BIT));

        vaddr_t entry_syscall = (vaddr_t)x86_syscall_entry;
        uint64_t star = 0;
        star |= (uint64_t)KERNEL_CODE_SELECTOR << 32;
        star |= (uint64_t)(UNUSED_SELECTOR | 3) << 48;
        trace_debug(MODULE, "star = %p", star);
        msr_set_64(MSR_STAR, star);
        msr_set_64(MSR_LSTAR, entry_syscall);
        msr_set_64(MSR_SYSCALL_MASK, X86_EFLAGS_AC | X86_EFLAGS_DF | X86_EFLAGS_IF | X86_EFLAGS_TF);
    }
#else
    return;
#endif
}

status_t syscall_arch_init(boot_params_t *bp)
{
    ENTER_FUNC("%p", bp);
    syscall_per_cpu_init(bp);
    return ivt_set_handler(EXC_SYSCALL, syscall_arch_handler);
}
