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
    UART_write_fstr(COM1, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\r\n", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    UART_write_fstr(COM1, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x\r\n", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);

    // do something here
}
void writeRegisters(Registers *regs)
{
    log_debug("debug", "debug");
    log_debug("debug", "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    log_debug("debug", "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);

    // do something here
}

void DoubleFault(Registers *regs)
{
    log_debug("debug", "debug");
    log_debug("debug", "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    log_debug("debug", "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);
}

void HALInit()
{
    i686_GDTInitialize();
    i686_IDTInitialize();

    i686_GDTLoad();

    i686_IDTLoad();

    i686_ISRInitialize();
    i686_IRQInitialize();

    i686_ISRRegisterHandler(1, writeRegisters);
    i686_ISRRegisterHandler(3, breakpoint);
    i686_ISRRegisterHandler(8, DoubleFault);
}