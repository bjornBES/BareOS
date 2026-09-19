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

#include "kernel.h"
#include "kernel/isr/isr.h"
#include "kernel/cpuid/cpuid.h"
#include "kernel/msr/msr.h"
#include "kernel/acpi/apic/apic.h"

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
    log_debug("breakpoint", "breakpoint\n");
    frame_arch_dump_frame(frame);
    return KERRNO_SUCCESSES;
}

status_t write_registers(intr_frame_t *regs)
{
    log_debug("DEBUG", "======== DEBUG ========");
    log_debug(MODULE, "from cpu %d", cpu_arch_get_current());
    frame_arch_dump_frame(regs);
    log_debug("DEBUG", "======== DEBUG ========");
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

    log_info(MODULE, "========== PAGE FAULT ==========");
    frame_arch_dump_frame(regs);
    log_info(MODULE, "\t{ cr2 = %016p }", cr2);
    log_info(MODULE, "========== PAGE FAULT ==========");

    KERNEL_PANIC("PF", "KERNEL GOT a PF at %p", cr2);
    FUNC_NOT_IMPLEMENTED();
    return ENOSYS;
}

x86_arch_data_t arch_runtime_data;

__init void arch_setup(boot_params_t *boot_params)
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

    cpuid_regs regs_leaf1;
    cpuid(0x01, 0, &regs_leaf1);
    cpu_t *cpu;
    uint32_t apic_id = 0;
    if (BIT_GET(regs_leaf1.ecx, 21) == 1)
    {
        cpuid_regs regs_leaf0b;
        cpuid(0x0B, 0, &regs_leaf0b);
        apic_id = regs_leaf0b.edx;
        log_info(MODULE, "x2apic = %u", apic_id);
    }
    else
    {
        apic_id = BIT_GET_RANGE(regs_leaf1.ebx, 24, 31);
        log_info(MODULE, "xapic = %u", apic_id);
    }
    cpu = cpu_arch_get(apic_id);
    cpu->cpuid.leaf_0x1_0[0] = *((leaf_0x1_0_t *)((void *)&regs_leaf1));
    log_info(MODULE, "cpu = %p", cpu);

    gdt_initialize(&cpu->gdtr, cpu->gdt_table);
    tss_initialize(&cpu->tss, cpu->gdt_table, TSS_INDEX);

    gdt_load(&cpu->gdtr, cpu->gdt_table);

    tss_load(TSS_SELECTOR);

    idt_load();

    ivt_set_handler(EXC_DEBUG, write_registers);
    ivt_set_handler(EXC_BREAKPOINT, breakpoint);
    ivt_set_handler(EXC_DF, double_fault);
    ivt_set_handler(EXC_GP, general_protection_fault);
    ivt_set_handler(EXC_FAULT, page_fault);

    // check CPUID.0x01:EDX[5] MSR
    arch_runtime_data.has_msr = cpu->cpuid.leaf_0x1_0[0].msr;

    // check CPUID.0x01:EDX[3] PSE
    arch_runtime_data.paging.pse = cpu->cpuid.leaf_0x1_0[0].pse;

    // check CPUID.0x01:EDX[6] PAE
    arch_runtime_data.paging.pae = cpu->cpuid.leaf_0x1_0[0].pae;

    // check CPUID.0x01:EDX[13] Global bit in paging
    arch_runtime_data.paging.global = cpu->cpuid.leaf_0x1_0[0].pge;

    // check CPUID.0x01:EDX[16] PAT
    arch_runtime_data.paging.pat = cpu->cpuid.leaf_0x1_0[0].pat;

    // check CPUID.0x01:EDX[17] PSE_36 (supports the 36-Bit Page Size Extension which enables 4-MByte)
    arch_runtime_data.paging.pse_36 = cpu->cpuid.leaf_0x1_0[0].pse36;

    cpuid_regs regs_leaf7_0;
    cpuid(0x07, 0, &regs_leaf7_0);
    cpu->cpuid.leaf_0x7_0[0] = *((leaf_0x7_0_t *)((void *)&regs_leaf7_0));

    // check CPUID.0x07.0x00:EAX[31:0] MAX_SUBLEAF
    if (cpu->cpuid.leaf_0x7_0[0].leaf7_n_subleaves >= 0x00)
    {
        // check CPUID.0x07.0x00:ECX[3] PKU (protection keys for user-mode pages)
        arch_runtime_data.paging.has_user_pke = cpu->cpuid.leaf_0x7_0[0].pku;

        // check CPUID.0x07.0x00:ECX[16] LA57 (57-bit linear addresses and fivelevel paging)
        arch_runtime_data.paging.la47 = cpu->cpuid.leaf_0x7_0[0].la57;

        // check CPUID.0x07.0x00:ECX[31] PKS (protection keys for supervisormode pages)
        arch_runtime_data.paging.has_super_pke = cpu->cpuid.leaf_0x7_0[0].pks;
    }

    cpuid_regs regs_leaf_ext0;
    cpuid(0x80000000, 0, &regs_leaf_ext0);
    // check CPUID.0x80000000:EAX Maximum Input Value for Extended Function CPUID Information
    uint32_t max_ext_subleaf = regs_leaf_ext0.eax;

    if (max_ext_subleaf >= 0x80000001)
    {
        cpuid_regs regs_leaf_ext1;
        cpuid(0x80000001, 0, &regs_leaf_ext1);

        // check CPUID.0x80000001:EDX[26] 1 GB pages
        arch_runtime_data.paging.huge_pdpt = BIT_GET(regs_leaf_ext1.edx, 26);

        // check CPUID.0x80000001:EDX[20] EXECUTE_DIS
        arch_runtime_data.paging.has_nx = BIT_GET(regs_leaf_ext1.edx, 20);
        if (arch_runtime_data.paging.has_nx == 1)
        {
            uint64_t efer = msr_get_64(MSR_EFER) | BIT(MSR_EFER_NX_BIT);
            msr_set_64(MSR_EFER, efer);
        }

        // check CPUID.0x80000001:EDX[29] intel64
        arch_runtime_data.long_mode = BIT_GET(regs_leaf_ext1.edx, 29);
        arch_runtime_data.paging.paging_64 = arch_runtime_data.long_mode;
    }

    if (max_ext_subleaf >= 0x80000008)
    {
        cpuid_regs regs_leaf_ext8;
        cpuid(0x80000008, 0, &regs_leaf_ext8);

        // check CPUID.0x80000008:EAX[7:0] PHYS_ADDR_SIZE
        arch_runtime_data.paging.max_phys = BIT_GET_RANGE(regs_leaf_ext8.eax, 0, 7);
    }

    mmu_arch_init(boot_params);
    mmu_arch_disable_prints();
    log_debug(MODULE, "max_phys = 0x%x/%d", arch_runtime_data.paging.max_phys, arch_runtime_data.paging.max_phys);
    log_debug(MODULE, "pse = 0x%x", arch_runtime_data.paging.pse);
    log_debug(MODULE, "pae = 0x%x", arch_runtime_data.paging.pae);
    log_debug(MODULE, "pat = 0x%x", arch_runtime_data.paging.pat);
    log_debug(MODULE, "pse_36 = 0x%x", arch_runtime_data.paging.pse_36);
    log_debug(MODULE, "paging_64 = 0x%x", arch_runtime_data.paging.paging_64);
    log_debug(MODULE, "la47 = 0x%x", arch_runtime_data.paging.la47);
    log_debug(MODULE, "huge_pdpt = 0x%x", arch_runtime_data.paging.huge_pdpt);
    log_debug(MODULE, "global = 0x%x", arch_runtime_data.paging.global);
    log_debug(MODULE, "has_nx = 0x%x", arch_runtime_data.paging.has_nx);
    log_debug(MODULE, "has_user_pke = 0x%x", arch_runtime_data.paging.has_user_pke);
    log_debug(MODULE, "has_super_pke = 0x%x", arch_runtime_data.paging.has_super_pke);

    log_debug(MODULE, "max_ext_subleaf = 0x%x", max_ext_subleaf);

    boot_params_t *bp;
    {
        bp = kmalloc(sizeof(boot_params_t));
        vaddr_t virt_bootParams = ((vaddr_t)boot_params + PAGE_SIZE);
        mmu_map_region(&kernel_page, PAGE_SIZE, 0, sizeof(boot_params_t), kernel_text_flags);
        memcpy(bp, (void *)virt_bootParams, sizeof(boot_params_t));
        mmu_free_region(&kernel_page, virt_bootParams, sizeof(boot_params_t));

        log_debug(MODULE, "bootParams @ %p", bp);
        hexdump(bp, sizeof(boot_params_t), 16);
        hexdump(&bp->smp, sizeof(bp->smp), 16);

        bp->arch_runtime_data = &arch_runtime_data;
    }

    rsdt_parse(bp);

    madt_parse();

    // check CPUID.0x01:EDX[9] APIC
    if (BIT_GET(regs_leaf1.edx, 9) == 1)
    {
        status_t status = irq_initialize(apic_get_driver);
        if (status != KERRNO_SUCCESSES)
        {
            log_err(MODULE, "PIC time");
            goto pic_time; // sorry...
        }
    }
    else
    {
pic_time:
        FUNC_NOT_IMPLEMENTED();
    }

    hpet_parse();

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

    kernel_early_main(bp);

    while (true)
    {
        ;
    }
}
