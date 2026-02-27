/*
 * File: isr.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include "isr.h"
#include "idt.h"
#include "gdt.h"
#include "i686.h"
#include "debug.h"

#include <IO.h>
#include <stdio.h>
#include <stdint.h>

#define MODULE "ISR"

ISRHandler _ISRHandlers[256];

static const char *const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""};

void i686_ISRInitializeGates();

void i686_ISRInitialize()
{
    i686_ISRInitializeGates();
    for (int i = 0; i < 256; i++)
        i686_IDTEnableGate(i);

    i686_IDTDisableGate(0x80);
}

void __attribute__((cdecl)) i686_ISRHandler(Registers *regs)
{
    if (_ISRHandlers[regs->interrupt] != NULL)
    {
        _ISRHandlers[regs->interrupt](regs);
    }
    else if (regs->interrupt >= 32)
    {
        log_err(MODULE, "Unhandled interrupt %d!", regs->interrupt);
    }
    else
    {
        log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);
        log_crit(MODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
        log_crit(MODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);
        log_crit(MODULE, "KERNEL PANIC!");

        printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
        printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x\n", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);
        printf("KERNEL PANIC!\n");
        i686_Panic();
    }
}

void i686_ISRRegisterHandler(int interrupt, ISRHandler handler)
{
    _ISRHandlers[interrupt] = handler;
    i686_IDTEnableGate(interrupt);
}
