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
#include "kernel/frame/frame.h"
#include "type_arch.h"

#include "asm/vectors_arch.h"
#include "asm/cpu_arch.h"

#include "debug/debug.h"
#include "ivt/ivt.h"
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

void stack_trace(uint32_t max_frames)
{
    // ENTER_FUNC("%u, %p", max_frames, regs);
    vaddr_t bp;
    inline_asm("mov %0, rbp" : "=r"(bp));

    typedef struct
    {
        vaddr_t bp;
        vaddr_t ip;
    } stack_frame_t;

    stack_frame_t *frame = (stack_frame_t *)bp;

    trace_debug(MODULE, "Stack trace:");
    cpu_t *cpu = cpu_arch_get_current();
    for (uint32_t i = 0; i < max_frames; i++)
    {
        // sanity check — bail if EBP looks invalid
        // trace_debug(MODULE, "  [%u] ip = %p, bp = %p, cpu->kernel_stack = %p", i, frame->ip, frame->bp, cpu->kernel_stack);
        // trace_debug(MODULE, "here 1");
        if (cpu->kernel_stack == (frame->bp + 8))
        {
            break;
        }
        // trace_debug(MODULE, "here 2");
        if (!frame || frame->ip == 0)
        {
            break;
        }

        trace_debug(MODULE, "  [%u] ip = %p, bp = %p", i, frame->ip, frame->bp);
        frame = (stack_frame_t *)frame->bp;
    }
}

void isr_handler(intr_frame_t *frame)
{
    {
        if ((frame->interrupt >= 0 && frame->interrupt < IPI_END_PRINTABLE) || (frame->interrupt > CPU_TIMER_VECTOR && frame->interrupt <= MAX_VECTOR))
        {
            ENTER_FUNC("%p", frame);
        }
        if (frame->interrupt < EXC_END)
        {
            stack_trace(8);
        }
    }
    inline_asm("cli");

    interrupt_vector_t vector = frame->interrupt;

    if (ivt_handler(vector, frame) != 0)
    {
        frame_arch_dump_frame(frame);

        log_crit(MODULE, "Unhandled exception %d %s 0x%x", vector, exception_names[vector], frame->error);
        KERNEL_PANIC(MODULE, "Unhandled exception %d", vector);
    }

    inline_asm("sti");
}
