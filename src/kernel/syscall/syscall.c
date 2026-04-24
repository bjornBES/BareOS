/*
 * File: syscall.c
 * File Created: 22 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "syscall.h"

#include "debug/debug.h"

#include "task/process.h"
#include "arch/x86/isr.h"
#include "task/signal.h"

typedef struct
{
    int number;
    syscall_handler handler;
    pledge_flags_t pledge;
} syscall_function_info;


#define X(nr, name, sysname, sysnameupper, ret, ...) extern ret sys_##name(syscall_registers *regs);
#include "syscall_table.tbl"
#undef X

#define X(nr, name, sysname, sysnameupper, ret, pledge, ...) [nr] = {nr, sys_##name, pledge},
static const syscall_function_info syscall_table[] = {
#include "syscall_table.tbl"
};
#undef X

#define MODULE "SYSCALL"

#define ENOSYS -1

#define X(nr, name, sysname, sysnameupper, ret, ...) SYS_##sysnameupper = nr,

typedef enum
{
#include "syscall_table.tbl"
    SYSCALL_COUNT
} syscall_nr_t;

#undef X

void syscall_handler_func(Registers *regs)
{
    log_info(MODULE, "System call %d from pid %u", regs->ax, current_process->pid);
    log_info(MODULE, "  ax=%x  bx=%x  cx=%x  dx=%x  si=%x  di=%x", regs->ax, regs->bx, regs->cx, regs->dx, regs->si, regs->di);
    log_info(MODULE, "  sp=%x  bp=%x  pc=%x  flags=%x  cs=%x  ds=%x ss=%x", regs->sp, regs->bp, regs->pc, regs->flags, regs->cs, regs->ds, regs->ss);
    log_info(MODULE, "# of syscalls %u", SYSCALL_COUNT);
    syscall_registers sys_regs;
    sys_regs.ax = regs->ax;
    sys_regs.bx = regs->bx;
    sys_regs.cx = regs->cx;
    sys_regs.dx = regs->dx;
    sys_regs.di = regs->di;
    sys_regs.si = regs->si;
    
    uint32_t nr = regs->ax;
    syscall_function_info info = syscall_table[nr];
    log_info(MODULE, "handler address %p", info.handler);

    if (info.pledge != 0 && !pledge_check(info.pledge))
    {
        fprintf(VFS_FD_DEBUG, "[%u]: pledge \"%s\", syscall %u,", current_process->pid, pledge_get_missing(info.pledge), nr);
        process_kill(current_process->pid, SIGKILL);
        return;
    }

    if (nr < SYSCALL_COUNT && info.handler)
    {
        regs->ax = info.handler(&sys_regs);
    }
    else
    {
        log_crit(MODULE, "Unhandled System call %d", regs->ax);
        log_crit(MODULE, "  ax=%x  bx=%x  cx=%x  dx=%x  si=%x  di=%x", regs->ax, regs->bx, regs->cx, regs->dx, regs->si, regs->di);
        log_crit(MODULE, "  sp=%x  bp=%x  pc=%x  flags=%x  cs=%x  ds=%x ss=%x", regs->sp, regs->bp, regs->pc, regs->flags, regs->cs, regs->ds, regs->ss);
        regs->ax = ENOSYS;
    }
}

void syscall_init()
{
    x86_isr_register_handler(0x80, syscall_handler_func);
}
