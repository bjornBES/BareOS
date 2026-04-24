/*
 * File: hal.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "hal.h"

#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"

#include "arch/x86/irq.h"
#include "arch/x86/isr.h"

#include "debug/debug.h"
#include "drivers/serial/UART/UART.h"

#define MODULE "HAL"

void breakpoint(Registers *regs)
{
    char buf[2048];
    x86_ISRFormatRegisters(regs, buf, "\n", "  ");
    log_debug("breakpoint", "breakpoint\n");
    log_debug("breakpoint", "%s", buf);

    // do something here
}
void writeRegisters(Registers *regs)
{
    char buf[2048];
    x86_ISRFormatRegisters(regs, buf, "\n", "  ");
    log_debug("debug", "debug");
    log_debug("debug", "%s", buf);
    // do something here
}

void DoubleFault(Registers *regs)
{
    ISR_show_stack_trace(10, regs);
    char buf[2048];
    x86_ISRFormatRegisters(regs, buf, "\n", "  ");
    log_err("double", "double fault");
    log_debug("double", "%s", buf);
    // panic("double fault", __FILE__, __LINE__, "double fault");
}
void GeneralProtectionFault(Registers *regs)
{
    char buf[2048];
    x86_ISRFormatRegisters(regs, buf, "\n", "  ");
    log_err("GPF", "General Protection Fault %x", regs->error);
    log_debug("GPF", "%s", buf);
    panic("GPF", __FILE__, __LINE__, "KERNEL GOT a GPF from %u ss", regs->error);
}

void HALInit()
{
    x86_GDT_initialize();
    UART_write_fstr(COM1, "GDT is done\r\n");
    log_debug(MODULE, "GDT is done");

    x86_IDT_initialize();
    UART_write_fstr(COM1, "IDT is done\r\n");
    log_debug(MODULE, "IDT is done");

    x86_TSS_initialize();
    UART_write_fstr(COM1, "TSS is done\r\n");
    log_debug(MODULE, "TSS is done");
    
    x86_GDT_load();
    UART_write_fstr(COM1, "GDT is loaded\r\n");
    log_debug(MODULE, "GDT is loaded");
    
    x86_IDT_load();
    UART_write_fstr(COM1, "IDT is loaded\r\n");
    log_debug(MODULE, "IDT is loaded");
    
    x86_ISRInitialize();
    UART_write_fstr(COM1, "ISR init\r\n");
    log_debug(MODULE, "ISR init");

    x86_irq_initialize();
    UART_write_fstr(COM1, "IRQ init\r\n");
    log_debug(MODULE, "IRQ init");

    x86_isr_register_handler(1, writeRegisters);
    x86_isr_register_handler(3, breakpoint);
    x86_isr_register_handler(8, DoubleFault);
    x86_isr_register_handler(13, GeneralProtectionFault);
}