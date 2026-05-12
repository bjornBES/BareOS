/*
 * File: exception.c
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "exception.h"
#include "libs/stdio.h"
#include "kernel.h"
#include "isr.h"

#define MODULE "exception"

kernel_handler exception_handlers_kernel[256];
arch_handler exception_handlers_arch[256];

int exception_handler(registers *regs)
{
    int interrupt = regs->interrupt;
    if (exception_handlers_arch[interrupt] != NULL)
    {
        kernel_handler kern_handler = exception_handlers_kernel[interrupt];
        exception_handlers_arch[interrupt](regs, kern_handler);
    }
    return RETURN_GOOD;
}

void exception_register_arch_handler(int interrupt, arch_handler handler)
{
    x86_isr_register_handler(interrupt);
    exception_handlers_arch[interrupt] = handler;
}

void exception_register_kernel_handler(int interrupt, kernel_handler handler)
{
    exception_handlers_kernel[interrupt] = handler;
}

void exception_format_registers(registers *regs, char *buf)
{
    int index = sprintf(buf, "ax=0x%llx bx=0x%llx cx=0x%llx dx=0x%llx si=0x%llx di=0x%llx\nsp=0x%llx bp=0x%llx pc=0x%04x:0x%llx flags=0x%x cs=0x%x ds=0x%x ss=0x%x",
                        regs->ax, regs->bx, regs->cx, regs->dx, regs->si, regs->di,
                        regs->sp, regs->bp, regs->cs, regs->pc, regs->flags, regs->cs, regs->ds, regs->ss);
    buf[index] = '\0';
}

void exception_init()
{
    x86_isr_initialize();
}
