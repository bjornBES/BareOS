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
#include "drivers/serial/UART/UART.h"

#include "debug/debug.h"
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

typedef struct {
    uint32_t ebp;
    uint32_t eip;
} stack_frame_t;

void stack_trace(uint32_t max_frames) {
    stack_frame_t *frame;

    // read current EBP directly
    __asm__ volatile("mov %%ebp, %0" : "=r"(frame));

    UART_write_fstr(COM1, "Stack trace:\r\n");
    for (uint32_t i = 0; i < max_frames; i++) {
        // sanity check — bail if EBP looks invalid
        if (!frame || (uint32_t)frame < 0x100000 || frame->eip == 0)
            break;

        UART_write_fstr(COM1, "  [%u] EIP = 0x%08x  EBP = 0x%08x\r\n",
                i, frame->eip, frame->ebp);

        frame = (stack_frame_t *)frame->ebp;
    }
}

void __attribute__((cdecl)) i686_ISRHandler(Registers *regs)
{
    if (_ISRHandlers[regs->interrupt] != NULL)
    {
        _ISRHandlers[regs->interrupt](regs);
    }
    else if (regs->interrupt >= 32)
    {
        UART_write_fstr(COM1, "Unhandled interrupt %d\r\n", regs->interrupt);
        log_err(MODULE, "Unhandled interrupt %d!", regs->interrupt);
    }
    else
    {
        stack_trace(4);
        UART_write_fstr(COM1, "Unhandled exception %d\r\n", regs->interrupt/* , g_Exceptions[regs->interrupt] */);
        UART_write_fstr(COM1, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\r\n", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
        UART_write_fstr(COM1, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x\r\n", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);
        
        log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);
        log_crit(MODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
        log_crit(MODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);
        
        printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
        printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x\n", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);
        KernelPanic("ISR", "Unhandled exception %d", regs->interrupt);
    }
}

void i686_ISRRegisterHandler(int interrupt, ISRHandler handler)
{
    _ISRHandlers[interrupt] = handler;
    i686_IDTEnableGate(interrupt);
}
