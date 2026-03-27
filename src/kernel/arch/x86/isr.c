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
#include "x86.h"
#include "drivers/serial/UART/UART.h"

#include "debug/debug.h"
#include "libs/IO.h"
#include "libs/stdio.h"

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

void x86_ISRInitializeGates();

void x86_ISRInitialize()
{
    x86_ISRInitializeGates();
    for (int i = 0; i < 256; i++)
        x86_IDT_enable_gate(i);

    x86_IDT_disable_gate(0x80);
}

typedef struct {
    void* bp;
    void* ip;
} stack_frame_t;

void stack_trace(uint32_t max_frames) {
    stack_frame_t *frame;

    // read current EBP directly
    __asm__ volatile("mov %0, rbp" : "=r"(frame));

    UART_write_fstr(COM1, "Stack trace:\r\n");
    for (uint32_t i = 0; i < max_frames; i++) {
        // sanity check — bail if EBP looks invalid
        if (!frame || (uint64_t)frame < 0x100000 || frame->ip == 0)
            break;

        UART_write_fstr(COM1, "  [%u] RIP = 0x%08x  RBP = 0x%08x\r\n",
                i, frame->ip, frame->bp);

        frame = (stack_frame_t *)frame->bp;
    }
}

void x86_ISR_handler(Registers *regs)
{
    UART_write_fstr(COM1, "interrupt is %u\r\n", regs->interrupt);
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
        UART_write_fstr(COM1, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\r\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        UART_write_fstr(COM1, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x\r\n", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        
        log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);
        log_crit(MODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        log_crit(MODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        
        printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x ss=%x\n", regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        KernelPanic("ISR", "Unhandled exception %d", regs->interrupt);
    }
}

void x86_isr_register_handler(int interrupt, ISRHandler handler)
{
    _ISRHandlers[interrupt] = handler;
    x86_IDT_enable_gate(interrupt);
}
