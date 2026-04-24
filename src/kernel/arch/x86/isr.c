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
#include "arch/x86/paging/x86_paging.h"

#include "debug/debug.h"
#include "libs/IO.h"
#include "libs/stdio.h"

#include <printf_driver/printf.h>
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

typedef struct
{
    void *bp;
    void *ip;
} stack_frame_t;

void ISR_show_stack_trace(uint32_t max_frames, Registers *regs)
{
    stack_frame_t *frame = (stack_frame_t *)regs->bp;

    log_debug("STACK", "Stack trace:");
    for (uint32_t i = 0; i < max_frames; i++)
    {
        // sanity check — bail if EBP looks invalid
        if (!frame || (uint32_64)frame < 0x100000 || frame->ip == 0)
            break;

        log_debug("STACK", "  [%u] PC = 0x%08x  BP = 0x%08x",
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
        char buf[2048];
        x86_ISRFormatRegisters(regs, buf, "\r\n", "  ");
        ISR_show_stack_trace(4, regs);
        UART_write_fstr(COM1, "Unhandled exception %d\r\n", regs->interrupt /* , g_Exceptions[regs->interrupt] */);
        UART_write_fstr(COM1, "%s", buf);

        log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);
        log_crit(MODULE, "%s", buf);

        // printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        // printf("%s", buf);
        KernelPanic("ISR", "Unhandled exception %d", regs->interrupt);
    }
}

void x86_isr_register_handler(int interrupt, ISRHandler handler)
{
    _ISRHandlers[interrupt] = handler;
    x86_IDT_enable_gate(interrupt);
}

void x86_ISRFormatRegisters(Registers *regs, char *buf, char *postfix, char *prefix)
{
#ifdef __x86_64__
    sprintf(buf, "%sax=0x%llx bx=0x%llx cx=0x%llx dx=0x%llx si=0x%llx di=0x%llx%s%ssp=0x%llx bp=0x%llx pc=0x%04x:0x%llx flags=0b%llb cs=0x%x ds=0x%x ss=0x%x%s",
            prefix, regs->ax, regs->bx, regs->cx, regs->dx, regs->si, regs->di, postfix, prefix,
            regs->sp, regs->bp, regs->cs, regs->pc, regs->flags, regs->cs, regs->ds, regs->ss, postfix);
#endif
}
