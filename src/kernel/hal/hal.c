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

#include "arch/i686/gdt.h"
#include "arch/i686/idt.h"

#include "arch/i686/irq.h"
#include "arch/i686/isr.h"

#include "debug/debug.h"
#include "drivers/serial/UART/UART.h"

void breakpoint(Registers *regs)
{
    UART_write_fstr(COM1, "breakpoint\r\n");
    UART_write_fstr(COM1, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\r\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    UART_write_fstr(COM1, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x\r\n", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

    // do something here
}
void writeRegisters(Registers *regs)
{
    log_debug("debug", "debug");
    log_debug("debug", "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    log_debug("debug", "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

    // do something here
}

void DoubleFault(Registers *regs)
{
    log_debug("debug", "debug");
    log_debug("debug", "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    log_debug("debug", "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
}

void HALInit()
{
    i686_GDTInitialize();
    UART_write_fstr(COM1, "GDT is done\r\n");
    i686_IDTInitialize();
    UART_write_fstr(COM1, "IDT is done\r\n");
    
    i686_GDTLoad();
    UART_write_fstr(COM1, "GDT is loaded\r\n");

    i686_tss_initialize();
    UART_write_fstr(COM1, "TSS is done\r\n");
    
    i686_IDTLoad();
    UART_write_fstr(COM1, "IDT is loaded\r\n");
    
    i686_ISRInitialize();
    UART_write_fstr(COM1, "ISR init\r\n");
    i686_irq_initialize();
    UART_write_fstr(COM1, "IRQ init\r\n");

    i686_isr_register_handler(1, writeRegisters);
    i686_isr_register_handler(3, breakpoint);
    i686_isr_register_handler(8, DoubleFault);
}