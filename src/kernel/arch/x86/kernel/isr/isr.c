/*
 * File: isr.c
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "entry/desc/idt/idt.h"
#include "kernel/ivt/ivt.h"
#include "type_arch.h"

#include "debug/debug.h"
#include "kernel.h"

#include <defs.h>
#include <types.h>

#define MODULE "x86-isr"

static const char *const exception_names[] = {
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

extern void isr_initialize_gates();

void isr_initialize()
{
    isr_initialize_gates();
    for (int i = 0; i < 256; i++)
    {
        idt_enable_gate(i);
    }

    idt_disable_gate(0x80);
}

void isr_handler(intr_frame_t *frame)
{
    inline_asm("cli");

    interrupt_vector vector = frame->interrupt;

    if (ivt_arch_handler(vector, frame) != 0)
    {
        ivt_dump_frame(frame);

        log_crit(MODULE, "Unhandled exception %d %s 0x%x", vector, exception_names[vector], frame->error);
        KERNEL_PANIC(MODULE, "Unhandled exception %d", vector);
    }

    inline_asm("sti");
}
