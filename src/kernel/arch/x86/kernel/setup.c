/*
 * File: setup.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "gdt.h"
#include "task/tss.h"
#include "idt.h"

#include "irq.h"

#include "exceptions/isr.h"
#include "exceptions/exception.h"
#include "syscall.h"

#include "apic/apic.h"
#include "kernel/ACPI/ACPI.h"
#include "kernel/ACPI/FADT.h"

#include "time/timer.h"
#include "kernel.h"

#include "memory/paging/paging.h"

#include "debug/debug.h"
#include <boot/bootparams.h>
#include "boot/setup_boot.h"
#include "smp/smp.h"

#define MODULE "X86"

extern void kernel_breakpoint(registers *regs);

void breakpoint(registers *regs, kernel_handler null)
{
    char buf[2048];
    x86_isr_format_registers(regs, buf, "\n", "  ");
    log_debug("breakpoint", "breakpoint\n");
    log_debug("breakpoint", "%s", buf);
    
    kernel_breakpoint(regs);
    
    // do something here
}
void writeRegisters(registers *regs, kernel_handler null)
{
    char buf[2048];
    x86_isr_format_registers(regs, buf, "\n", "  ");
    log_debug("debug", "debug");
    log_debug("debug", "%s", buf);
    // do something here
}

void DoubleFault(registers *regs, kernel_handler null)
{
    isr_show_stack_trace(10, regs);
    char buf[2048];
    x86_isr_format_registers(regs, buf, "\n", "  ");
    log_err("double", "double fault");
    log_debug("double", "%s", buf);
    // panic("double fault", __FILE__, __LINE__, "double fault");
}
void GeneralProtectionFault(registers *regs, kernel_handler null)
{
    char buf[2048];
    x86_isr_format_registers(regs, buf, "\n", "  ");
    log_err("GPF", "General Protection Fault %x", regs->error);
    log_debug("GPF", "%s", buf);
    x86_GDT_dump_selector(regs->error);
    panic("GPF", __FILE__, __LINE__, "KERNEL GOT a GPF from %u ss", regs->error);
}

boot_params *setup_arch(boot_params *bootParams)
{
    x86_GDT_initialize();
    log_debug(MODULE, "GDT is done");

    x86_idt_initialize();
    log_debug(MODULE, "IDT is done");

    x86_GDT_load(&gdt_descriptor, gdt_table);
    log_debug(MODULE, "GDT is loaded");

    tss_load(x86_TSS_SEGMENT);

    x86_idt_load(kernel_idtr, kernel_idt_table);
    log_debug(MODULE, "IDT is loaded");

    x86_isr_initialize();
    log_debug(MODULE, "ISR init");

    x86_irq_initialize();
    log_debug(MODULE, "IRQ init");

    exception_init();
    
    exception_register_arch_handler(1, writeRegisters);
    exception_register_arch_handler(3, breakpoint);
    exception_register_arch_handler(8, DoubleFault);
    exception_register_arch_handler(13, GeneralProtectionFault);
    
    enable_interrupts();
    
    paging_init(bootParams);
    
    boot_params *bp;
    bp = boot_move_params(bootParams);
    log_debug(MODULE, "bootParams @ %p", bootParams);

    arch_syscall_init();

    ACPI_init(bp->acpi);
    FADT_init();

#if __x86_64__ || __i686__
    if (!irq_inject_driver(apic_get_driver()))
    {
        log_warn("MAIN", "APIC not avalble");
    }
#endif
    
    smp_init(bp);

    enable_interrupts();
    timer_init();


    return bp;
}