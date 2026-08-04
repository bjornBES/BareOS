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
#include "kernel/debug.h"
#include "kernel/asm/segment/gdt.h"
#include "kernel/cpu.h"
#include "kernel/x86.h"
#include "kernel/ivt.h"
#include "kernel/irq.h"
#include "kernel/mmu.h"
#include "VFS/vfs.h"

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
        x86_idt_enable_gate(i);
    }

    x86_idt_disable_gate(0x80);
}

typedef struct
{
    vaddr_t bp;
    vaddr_t ip;
} stack_frame_t;

extern int exception_handler(intr_frame_t *regs);

void stack_trace(uint32_t max_frames, intr_frame_t *regs)
{
    // ENTER_FUNC(MODULE, "%u, %p", max_frames, regs);
    stack_frame_t *frame = (stack_frame_t *)regs->bp;

    page_table_t page_dir;
    mmu_arch_current_table(&page_dir);
    int state = mmu_arch_is_present(&page_dir, PAGE_ALIGN_UP(regs->bp));
    if (state != RETURN_GOOD)
    {
        return;
    }

    log_debug(MODULE, "Stack trace:");
    for (uint32_t i = 0; i < max_frames; i++)
    {
        // sanity check — bail if EBP looks invalid
        if (!frame || frame->ip == 0)
        {
            break;
        }

        log_debug(MODULE, "  [%u] ip = %p, bp = %p", i, frame->ip, frame->bp);
        if (mmu_arch_is_present(&page_dir, frame->bp) != RETURN_GOOD)
        {
            break;
        }
        frame = (stack_frame_t *)frame->bp;
    }
}

void x86_isr_handler(intr_frame_t *regs)
{
    irq_arch_disable();
    cpu_t *cpu = cpu_arch_get_current();
    if (cpu != NULL && cpu->current != NULL)
    {
        if (!cpu->current->is_idle_thread)
        {
            if (regs->interrupt != LAPIC_TIMER_VECTOR)
            {
                if (vfs_init_is_done)
                {
                    trace_with_id(4, LVL2, "got interrupt %u\n", regs->interrupt);
                }
                else
                {
                    log_debug(MODULE, "got interrupt %u from %u.%u", regs->interrupt, cpu->cpu_id, cpu->current->tid);
                }
            }
            else if (regs->interrupt == LAPIC_TIMER_VECTOR)
            {
                if (vfs_init_is_done)
                {
                    // trace_with_id(4, LVL2, "T\n");
                }
                else
                {
                    // log_debug(NO_MODULE, "T%u", cpu->apic_id);
                }
            }
        }
    }

    if (regs->interrupt < IRQ0)
    {
        stack_trace(4, regs);
    }
    
    if (exception_handler(regs) != RETURN_GOOD)
    {
        irq_arch_disable();
        ivt_dump_frame(regs);
        
        log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);
        KERNEL_PANIC("ISR", "Unhandled exception %d", regs->interrupt);
    }
}

void x86_isr_register_handler(int interrupt)
{
    x86_idt_enable_gate(interrupt);
}

