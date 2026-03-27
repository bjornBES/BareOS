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

#include "arch/x86/isr.h"

#define X(nr, name, ret, ...) \
    SYS_##name = nr,

typedef enum
{
#include "syscall_table.tbl"
    SYSCALL_COUNT
} syscall_nr_t;

#undef X

#define X(nr, name, ret, ...) extern ret sys_##name(syscall_registers *regs);
#include "syscall_table.tbl"
#undef X

#define X(nr, name, ret, ...) [nr] = sys_##name,
syscall_handler syscall_table[] = {
#include "syscall_table.tbl"
};
#undef X

#define MODULE "SYSCALL"

#define ENOSYS -1

void syscall_handler_func(Registers *regs)
{
    log_info(MODULE, "System call %d", regs->eax);
    log_info(MODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    log_info(MODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
    log_info(MODULE, "# of syscalls %u", SYSCALL_COUNT);
    syscall_registers sys_regs;
    sys_regs.eax = regs->eax;
    sys_regs.ebx = regs->ebx;
    sys_regs.ecx = regs->ecx;
    sys_regs.edx = regs->edx;
    sys_regs.edi = regs->edi;
    sys_regs.esi = regs->esi;

    uint32_t nr = regs->eax;
    if (nr < SYSCALL_COUNT && syscall_table[nr])
    {
        regs->eax = syscall_table[nr](&sys_regs);
    }
    else
    {
        log_crit(MODULE, "Unhandled System call %d", regs->eax);
        log_crit(MODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        log_crit(MODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        regs->eax = ENOSYS;
    }
}

void syscall_init()
{
    x86_isr_register_handler(0x80, syscall_handler_func);
}
