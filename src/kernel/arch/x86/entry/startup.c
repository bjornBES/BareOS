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
#include "x86_arch_data.h"

#include "asm/cpu_arch.h"
#include "asm/ivt_arch.h"
#include "asm/vectors_arch.h"
#include "asm/mmu_arch.h"
#include "asm/frame_arch.h"

#include "acpi/rsdt.h"
#include "acpi/hpet/hpet.h"
#include "acpi/madt/madt.h"

#include "entry/desc/gdt/gdt.h"
#include "entry/desc/idt/idt.h"
#include "debug/debug.h"

#include "mm/pmm/pmm.h"

#include "kernel.h"
#include "kernel/isr/isr.h"
#include "kernel/cpuid/cpuid.h"
#include "kernel/msr/msr.h"
#include "kernel/acpi/apic/apic.h"
#include "kernel/dev/pit/pit.h"

#include "resource/resource.h"

#include "module.h"
#include "memory.h"

#include "init.h"
#include "ivt/ivt.h"

#include <binary.h>
#include <defs.h>

#define MODULE "x86-setup"

extern void hexdump(void *ptr, size_t len, size_t size);

status_t breakpoint(intr_frame_t *frame)
{
    trace_debug("breakpoint", "breakpoint\n");
    frame_arch_dump_frame(frame);
    return KERRNO_SUCCESSES;
}

status_t write_registers(intr_frame_t *regs)
{
    trace_debug("DEBUG", "======== DEBUG ========");
    trace_debug(MODULE, "from cpu %d", cpu_arch_get_current());
    frame_arch_dump_frame(regs);
    trace_debug("DEBUG", "======== DEBUG ========");
    return KERRNO_SUCCESSES;
}

status_t double_fault(intr_frame_t *regs)
{
    frame_arch_dump_frame(regs);
    log_err("double", "double fault");

    FUNC_NOT_IMPLEMENTED();
    return ENOSYS;
}

status_t general_protection_fault(intr_frame_t *frame)
{
    log_err("GPF", "General Protection Fault 0x%x", frame->error);
    frame_arch_dump_frame(frame);
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
            return KERRNO_SUCCESSES;
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

status_t page_fault(intr_frame_t *regs)
{
    vaddr_t cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));

    trace_info(MODULE, "========== PAGE FAULT ==========");
    frame_arch_dump_frame(regs);
    trace_info(MODULE, "\t{ cr2 = %016p }", cr2);
    trace_info(MODULE, "========== PAGE FAULT ==========");

    KERNEL_PANIC("PF", "KERNEL GOT a PF at %p", cr2);
    FUNC_NOT_IMPLEMENTED();
    return ENOSYS;
}

static resource_t x86_exceptions = {
    .name = "x86 Exceptions",
    .start = 0,
    .end = 31,
    .flags = RES_FLAG_NONE,
    .type = RES_TYPE_RESERVED,
};

x86_arch_data_t arch_runtime_data;
boot_params_t *pre_boot_params;
boot_params_t *bp_arch;

__init void arch_setup(boot_params_t *boot_params)
{
    pre_boot_params = boot_params;
    resource_space_init(&io_space, RES_SPACE_IO, 0, 0xFFFF);
    resource_space_init(&irq_space, RES_SPACE_IRQ, 0, 255);

    resource_insert(&irq_space.root, &x86_exceptions);

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_ARCH);

    resource_space_init(&mem_space, RES_SPACE_MEM, 0, UINT64_MAX);

    ivt_set_handler(EXC_DEBUG, write_registers);
    ivt_set_handler(EXC_BREAKPOINT, breakpoint);
    ivt_set_handler(EXC_DF, double_fault);
    ivt_set_handler(EXC_GP, general_protection_fault);
    ivt_set_handler(EXC_FAULT, page_fault);

    mmu_arch_init(boot_params);
    mmu_arch_disable_prints();

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_CORE);

    trace_debug(MODULE, "max_phys = 0x%x/%d", arch_runtime_data.paging.max_phys, arch_runtime_data.paging.max_phys);
    trace_debug(MODULE, "pse = 0x%x", arch_runtime_data.paging.pse);
    trace_debug(MODULE, "pae = 0x%x", arch_runtime_data.paging.pae);
    trace_debug(MODULE, "pat = 0x%x", arch_runtime_data.paging.pat);
    trace_debug(MODULE, "pse_36 = 0x%x", arch_runtime_data.paging.pse_36);
    trace_debug(MODULE, "paging_64 = 0x%x", arch_runtime_data.paging.paging_64);
    trace_debug(MODULE, "la47 = 0x%x", arch_runtime_data.paging.la47);
    trace_debug(MODULE, "huge_pdpt = 0x%x", arch_runtime_data.paging.huge_pdpt);
    trace_debug(MODULE, "global = 0x%x", arch_runtime_data.paging.global);
    trace_debug(MODULE, "has_nx = 0x%x", arch_runtime_data.paging.has_nx);
    trace_debug(MODULE, "has_user_pke = 0x%x", arch_runtime_data.paging.has_user_pke);
    trace_debug(MODULE, "has_super_pke = 0x%x", arch_runtime_data.paging.has_super_pke);

    // trace_debug(MODULE, "max_ext_subleaf = 0x%x", max_ext_subleaf);

    {
        bp_arch = kmalloc(sizeof(boot_params_t));
        trace_debug(MODULE, "bootParams @ %p", bp_arch);
        vaddr_t virt_bootParams = ((vaddr_t)boot_params + PAGE_SIZE);
        mmu_map_region(&kernel_page, PAGE_SIZE, 0, sizeof(boot_params_t), kernel_text_flags);
        memcpy(bp_arch, (void *)virt_bootParams, sizeof(boot_params_t));
        mmu_free_region(&kernel_page, virt_bootParams, sizeof(boot_params_t));

        trace_debug(MODULE, "bootParams @ %p", bp_arch);
        pmm_insert_resource(bp_arch);
    }
    bp_arch->arch_runtime_data = &arch_runtime_data;

    rsdt_parse(bp_arch);
    
    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_POSTCORE);
    
    resource_dump(&mem_space.root);
    
    // check CPUID.0x01:EDX[25] SSE
    // check CPUID.0x01:EDX[26] SSE2
    
    // check CPUID.0x01:EDX[2] DE
    
    // check CPUID.0x01:EDX[7] MCE
    
    // check CPUID.0x01:ECX[21] x2APIC
    // check CPUID.0x01:EBX[23:16] APIC_ID_SPACE
    // check CPUID.0x01:EBX[31:24] INITIAL_APIC_ID
    
    // check CPUID.0x01:ECX[24] FXSR (FXSAVE/FXRSTOR)
    // check CPUID.0x01:ECX[26] XSAVE
    // check CPUID.0x01:ECX[27] OSXSAVE
    
    // check CPUID.0x07.0x00:EBX[0] FSGSBASE
    
    // check CPUID.0x07.0x00:EBX[7] SMEP (Supervisor-Mode Execution Prevention)
    // check CPUID.0x07.0x00:EBX[9] ENH_REP_MOVSB_STOSB (Enhanced REP MOVSB/STOSB)
    
    // check CPUID.0x07.0x00:EDX[29] ARCH_CAPABILITIES (IA32_ARCH_CAPABILITIES MSR)
    // check CPUID.0x07.0x00:EDX[30] CORE_CAPABILITIES (IA32_CORE_CAPABILITIES MSR)
    
    // check CPUID.0x07.0x01:EAX[2:0] SHA512, SM3, SM4 instructions
    // check CPUID.0x07.0x01:EAX[12:10] REP MOVSB STOSB CMPSB instructions
    
    // check CPUID.0x01:EDX[11] support the SYSENTER and SYSEXIT Instructions
    // check CPUID.0x80000001:EDX[11] syscall fast path
    // check CPUID.0x07.0x01:EAX[17] FRED
    
    // check CPUID.0x07.0x01:EAX[20] NMI_SRC
    
    // check CPUID.0x0D Processor Extended State
    
    // check CPUID.0x14 Processor Trace
    
    // check CPUID.0x16 Processor Frequency Information

    kernel_early_main(bp_arch);

    while (true)
    {
        ;
    }
}
