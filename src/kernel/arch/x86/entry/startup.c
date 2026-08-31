/*
 * File: startup.c
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "setup.h"

#include "desc/gdt/gdt.h"
#include "desc/idt/idt.h"
#include "debug/debug.h"

#include "asm/cpu_arch.h"
#include "asm/ivt_arch.h"
#include "asm/vectors_arch.h"

#include "module.h"

#include "kernel.h"

#include <defs.h>

#define MODULE "x86-setup"

int breakpoint(intr_frame_t *frame)
{
    log_debug("breakpoint", "breakpoint\n");
    ivt_dump_frame(frame);

    // kernel_breakpoint(regs);

    // do something here
    return 0;
}

int write_registers(intr_frame_t *regs)
{
    log_debug("DEBUG", "======== DEBUG ========");
    log_debug(MODULE, "from cpu %d", cpu_arch_get_current());
    ivt_dump_frame(regs);
    log_debug("DEBUG", "======== DEBUG ========");
    return 0;
}

int double_fault(intr_frame_t *regs)
{
    ivt_dump_frame(regs);
    log_err("double", "double fault");

    FUNC_NOT_IMPLEMENTED();
    return ENOSYS;
}

int general_protection_fault(intr_frame_t *frame)
{
    log_err("GPF", "General Protection Fault 0x%x", frame->error);
    ivt_dump_frame(frame);
    uint8_t table = BIT_GET_RANGE(frame->error, 1, 2);
    uint16_t selector = frame->error & ~0x3;
    if (table == 0b00)
    {
        gdt_dump_selector(selector);
        gdt_entry_t *entry = gdt_get_entry(selector);
        uint32_t checksum = entry->access + entry->base_high + entry->base_low + entry->base_middle + entry->flags + entry->limit_high + entry->limit_low;
        if (selector == 0 && checksum != 0)
        {
            gdt_set_entry(entry, 0, 0, 0, 0, 0);
            return 0;
        }
    }
    else if (table == 0b01 || table == 0b11)
    {
        idt_dump_selector(selector);
    }
    KERNEL_PANIC("GPF", "KERNEL GOT a GPF from %u ss", frame->error);

    FUNC_NOT_IMPLEMENTED();
    return ENOSYS;
}

uint8_t PF_times = 0;

int page_fault(intr_frame_t *regs)
{
    vaddr_t cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));

    log_info(MODULE, "========== PAGE FAULT ==========");
    ivt_dump_frame(regs);
    log_info(MODULE, "\t{ cr2 = %016p }", cr2);
    log_info(MODULE, "========== PAGE FAULT ==========");

    KERNEL_PANIC("PF", "KERNEL GOT a PF at %p", cr2);
    FUNC_NOT_IMPLEMENTED();
    return ENOSYS;
}

void arch_setup(boot_params_t *boot_params)
{
    // what do we know here?
    // - what some devices needs what drivers using cmdline
    // - where the kernel is
    // - we have the acpi location
    // - we have the systems memory map
    // - we know what video mode we use
    // - we are in 64 bit long mode or 32 bit pmode
    // - we know we can use cpuid
    // - we know we are i386+

    cpu_t *cpu = cpu_arch_get(0);
    gdt_initialize(&cpu->gdtr, cpu->gdt_table);
    tss_initialize(&cpu->tss, cpu->gdt_table, TSS_INDEX);

    gdt_load(&cpu->gdtr, cpu->gdt_table);

    tss_load(TSS_SELECTOR);

    ivt_arch_init();

    idt_load();

    ivt_arch_set_handler(EXC_DEBUG, write_registers);
    ivt_arch_set_handler(EXC_BREAKPOINT, breakpoint);
    ivt_arch_set_handler(EXC_DF, double_fault);
    ivt_arch_set_handler(EXC_GP, general_protection_fault);
    ivt_arch_set_handler(EXC_PF, page_fault);

    

    // check CPUID.0x01:EDX[25] SSE
    // check CPUID.0x01:EDX[26] SSE2

    // check CPUID.0x01:EDX[3] PSE
    // check CPUID.0x01:EDX[6] PAE
    // check CPUID.0x01:EDX[13] Global bit in paging
    // check CPUID.0x01:EDX[16] PAT
    // check CPUID.0x01:EDX[17] PSE_36 (supports the 36-Bit Page Size Extension which enables 4-MByte)

    // check CPUID.0x01:EDX[2] DE

    // check CPUID.0x01:EDX[7] MCE

    // check CPUID.0x01:EDX[9] APIC
    // check CPUID.0x01:ECX[21] x2APIC
    // check CPUID.0x01:EBX[23:16] APIC_ID_SPACE
    // check CPUID.0x01:EBX[31:24] INITIAL_APIC_ID

    // check CPUID.0x01:ECX[24] FXSR (FXSAVE/FXRSTOR)
    // check CPUID.0x01:ECX[26] XSAVE
    // check CPUID.0x01:ECX[27] OSXSAVE

    // check CPUID.0x07.0x00:EAX[31:0] MAX_SUBLEAF
    // check CPUID.0x07.0x00:EBX[0] FSGSBASE

    // check CPUID.0x07.0x00:EBX[7] SMEP (Supervisor-Mode Execution Prevention)
    // check CPUID.0x07.0x00:EBX[9] ENH_REP_MOVSB_STOSB (Enhanced REP MOVSB/STOSB)

    // check CPUID.0x07.0x00:ECX[3] PKU (protection keys for user-mode pages)
    // check CPUID.0x07.0x00:ECX[16] LA57 (57-bit linear addresses and fivelevel paging)
    // check CPUID.0x07.0x00:ECX[31] PKS (protection keys for supervisormode pages)

    // check CPUID.0x07.0x00:EDX[29] ARCH_CAPABILITIES (IA32_ARCH_CAPABILITIES MSR)
    // check CPUID.0x07.0x00:EDX[30] CORE_CAPABILITIES (IA32_CORE_CAPABILITIES MSR)

    // check CPUID.0x07.0x01:EAX[2:0] SHA512, SM3, SM4 instructions
    // check CPUID.0x07.0x01:EAX[12:10] REP MOVSB STOSB CMPSB instructions

    // check CPUID.0x07.0x01:EAX[17] FRED

    // check CPUID.0x07.0x01:EAX[20] NMI_SRC

    // check CPUID.0x0D Processor Extended State

    // check CPUID.0x14 Processor Trace

    // check CPUID.0x16 Processor Frequency Information

    // check CPUID.0x80000000:EAX Maximum Input Value for Extended Function CPUID Information

    // check CPUID.0x01:EDX[11] support the SYSENTER and SYSEXIT Instructions
    // check CPUID.0x80000001:EDX[11] syscall fast path
    // check CPUID.0x80000001:EDX[26] 1 GB pages
    // check CPUID.0x80000001:EDX[20] EXECUTE_DIS
    // check CPUID.0x80000001:EDX[29] intel64

    // check CPUID.0x80000008:EAX[7:0] PHYS_ADDR_SIZE

    while (true)
    {
        ;
    }
}
