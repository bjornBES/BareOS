/*
 * File: syscall.c
 * File Created: 22 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 16:17:51
 * Modified By: BjornBEs
 * -----
 */

#include "syscall.h"
#include <syscall/syscall_number.h>

#include "debug/debug.h"

#include "kernel/exceptions/exception.h"
#include "task/process.h"
#include "task/signal.h"
#include "libs/string.h"

#define X(nr, name, sysname, sysnameupper, ...) extern uintptr_t sys_##name(syscall_info *info, syscall_function_info *sys_info);
#include <syscall/syscall_table.tbl>
#undef X

#define X(nr, name, sysname, sysnameupper, ret, pledge, ...) [nr] = {nr, sys_##name, ret, pledge},
static const syscall_function_info syscall_table[] = {
#include <syscall/syscall_table.tbl>
};
#undef X

#define MODULE "SYSCALL"

#define ENOSYS -1

void syscall_handler_func(void *_info)
{
    syscall_info *info = (syscall_info*)_info;
    registers *regs = info->regs;
    fprintf(VFS_FD_DEBUG, "System call %d from pid %u\n", regs->ax, current_process->pid);
    /*
    log_info(MODULE, "  ax=%lx  bx=%lx  cx=%lx  dx=%lx  si=%lx  di=%lx  r8=%lx  r9=%lx", regs->ax, regs->bx, regs->cx, regs->dx, regs->si, regs->di, regs->r8, regs->r9);
    log_info(MODULE, "  r10=%lx  r11=%lx  r12=%lx  r13=%lx  r14=%lx  r15=%lx", regs->r10, regs->r11, regs->r12, regs->r13, regs->r14, regs->r15);
    log_info(MODULE, "  sp=%lx  bp=%lx  pc=%lx  flags=%b  cs=%x  ds=%x ss=%x", regs->sp, regs->bp, regs->pc, regs->flags, regs->cs, regs->ds, regs->ss);
    log_info(MODULE, "# of syscalls %u", SYSCALL_COUNT); */
/*     {
        uint64_t cr3;
        uint64_t kernel_paging = (uint64_t)kernel_page.page_dir - (uint64_t)(KERNEL_VIRT_BASE - KERNEL_PHYS_BASE);
        __asm__ volatile("mov %0, cr3" : "=r"(cr3));
        bool cr3IsKernel = cr3 == kernel_paging;
        if (!cr3IsKernel)
        {
            fprintf(VFS_FD_DEBUG, "active cr3 (%p) is (%u) users\n",
                    cr3, current_process->pid);
        }
        else
        {
            fprintf(VFS_FD_DEBUG, "active cr3 (%p) is kernels\n", cr3);
        }
    } */
/*     syscall_registers sys_regs;
    sys_regs.ax = regs->ax;
    sys_regs.bx = regs->bx;
    sys_regs.cx = regs->cx;
    sys_regs.dx = regs->dx;
    sys_regs.di = regs->di;
    sys_regs.si = regs->si;
    sys_regs.sp = regs->sp;
    sys_regs.bp = regs->bp;
    sys_regs.r8 = regs->r8;
    sys_regs.r9 = regs->r9;
    sys_regs.r10 = regs->r10; */

    uint32_t nr = info->sys_number;
    syscall_function_info syscall_info = syscall_table[nr];
    // log_info(MODULE, "handler address %p", syscall_info.handler);

    if (syscall_info.pledge != 0 && pledge_check(syscall_info.pledge) != RETURN_GOOD)
    {
        process_t *current = process_get_current();
        fprintf(VFS_FD_DEBUG, "[%u]: pledge \"%s\", syscall %u,\n", current->pid, pledge_get_missing(syscall_info.pledge), nr);
        signal_enqueue(current, SIGSYS);
        signal_try_call(info);
        return;
    }

    uintptr_t ret = 0;
    if (nr < SYSCALL_COUNT && syscall_info.handler)
    {
        ret = syscall_info.handler(info, &syscall_info);
        if (syscall_info.call_dose_return)
        {
        }
    }
    else
    {
        log_crit(MODULE, "Unhandled System call %d", regs->ax);
        log_crit(MODULE, "  ax=%x  bx=%x  cx=%x  dx=%x  si=%x  di=%x", regs->ax, regs->bx, regs->cx, regs->dx, regs->si, regs->di);
        log_crit(MODULE, "  sp=%x  bp=%x  pc=%x  flags=%x  cs=%x  ds=%x ss=%x", regs->sp, regs->bp, regs->pc, regs->flags, regs->cs, regs->ds, regs->ss);
        regs->ax = ENOSYS;
    }
    signal_try_call(info);
    regs->ax = ret;
    // fprintf(VFS_FD_DEBUG, "es");
}

void syscall_init()
{
    exception_register_kernel_handler(EXC_SYSCALL, syscall_handler_func);
}
