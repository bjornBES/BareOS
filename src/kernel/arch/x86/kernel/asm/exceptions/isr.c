/*
 * File: isr.c
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "isr.h"
#include "kernel/asm/ivt/idt.h"
#include "kernel/asm/segment/gdt.h"
#include "kernel/x86.h"
#include "kernel/ivt.h"

#include "debug/debug.h"
#include "kernel/io.h"
#include "stdio.h"

#include <printf_driver/printf.h>
#include <stdint.h>

#define MODULE "ISR"

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

void x86_isr_initialize_gates();

void x86_isr_initialize()
{
    x86_isr_initialize_gates();
    for (int i = 0; i < 256; i++)
    {
        x86_idt_enable_gate(kernel_idt_table, i);
    }

    x86_idt_disable_gate(kernel_idt_table, 0x80);
}

typedef struct
{
    void *bp;
    void *ip;
} stack_frame_t;

extern int exception_handler(intr_frame_t *regs);

void x86_isr_handler(intr_frame_t *regs)
{
    // log_debug(MODULE, "interrupt is %u flags = %064b", _int, regs->flags);
    // UART_write_fstr(COM1, "interrupt is %u\r\n", regs->interrupt);
    if (exception_handler(regs) != RETURN_GOOD)
    {
        ivt_dump_frame(regs);
        // UART_write_fstr(COM1, "Unhandled exception %d\r\n", regs->interrupt /* , g_Exceptions[regs->interrupt] */);
        // UART_write_fstr(COM1, "%s", buf);

        log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);

        // printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        // printf("%s", buf);
        KernelPanic("ISR", "Unhandled exception %d", regs->interrupt);
    }
}

void x86_isr_register_handler(int interrupt)
{
    x86_idt_enable_gate(kernel_idt_table, interrupt);
}

