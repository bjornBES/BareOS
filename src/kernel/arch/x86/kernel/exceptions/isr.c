/*
 * File: isr.c
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "isr.h"
#include "kernel/idt.h"
#include "kernel/gdt.h"
#include "kernel/x86.h"
#include "kernel/paging/paging.h"

#include "debug/debug.h"
#include "libs/IO.h"
#include "libs/stdio.h"

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
        x86_idt_enable_gate(kernel_idt_table, i);

    x86_idt_disable_gate(kernel_idt_table, 0x80);
}

typedef struct
{
    void *bp;
    void *ip;
} stack_frame_t;

void isr_show_stack_trace(uint32_t max_frames, registers *regs)
{
    stack_frame_t *frame = (stack_frame_t *)regs->bp;

    log_debug("STACK", "Stack trace:");
    for (uint32_t i = 0; i < max_frames; i++)
    {
        // sanity check — bail if EBP looks invalid
        if (!frame || (uint32_64)frame < 0x100000 || frame->ip == 0)
            break;

        log_debug("STACK", "  [%u] PC = %p  BP = %p",
                        i, frame->ip, frame->bp);

        frame = (stack_frame_t *)frame->bp;
    }
}

extern int exception_handler(registers *regs);
void x86_isr_handler(registers *regs)
{
    uint32_64 _int = regs->interrupt;
    // log_debug(MODULE, "interrupt is %u flags = %064b", _int, regs->flags);
    // UART_write_fstr(COM1, "interrupt is %u\r\n", regs->interrupt);
    if (exception_handler(regs) != RETURN_GOOD)
    {
        char buf[2048];
        x86_isr_format_registers(regs, buf, "\r\n", "  ");
        isr_show_stack_trace(4, regs);
        // UART_write_fstr(COM1, "Unhandled exception %d\r\n", regs->interrupt /* , g_Exceptions[regs->interrupt] */);
        // UART_write_fstr(COM1, "%s", buf);

        log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);
        log_crit(MODULE, "%s", buf);

        // printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        // printf("%s", buf);
        KernelPanic("ISR", "Unhandled exception %d", regs->interrupt);
    }
}

void x86_isr_register_handler(int interrupt)
{
    x86_idt_enable_gate(kernel_idt_table, interrupt);
}

void x86_isr_format_registers(registers *regs, char *buf, char *postfix, char *prefix)
{
#ifdef __x86_64__
    int index = sprintf(buf, "%sax=0x%llx bx=0x%llx cx=0x%llx dx=0x%llx si=0x%llx di=0x%llx%s%ssp=0x%llx bp=0x%llx pc=0x%04x:0x%llx flags=0b%llb cs=0x%x ds=0x%x ss=0x%x%s",
            prefix, regs->ax, regs->bx, regs->cx, regs->dx, regs->si, regs->di, postfix, prefix,
            regs->sp, regs->bp, regs->cs, regs->pc, regs->flags, regs->cs, regs->ds, regs->ss, postfix);
    buf[index] = '\0';
#endif
}
