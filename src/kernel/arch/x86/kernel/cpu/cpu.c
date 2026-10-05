/*
 * File: cpu.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/cpu_arch.h"
#include "asm/irq_arch.h"
#include "asm/mmu_arch.h"

#include "kernel/cpu/cpu.h"
#include "kernel/cpuid/cpuid.h"
#include "kernel/msr/msr.h"
#include "kernel/acpi/apic/lapic.h"

#include "entry/desc/idt/idt.h"

#include "mm/allocator/kstack_allocator.h"

#include "ivt/ivt.h"
#include "debug/debug.h"

#include "memory.h"

#include "init.h"

#include <config.h>
#include <binary.h>

#define MODULE "x86-cpu"

cpu_t cpus[CONFIG_MAX_CPUS] = {0};
cpu_t *bsp_cpu = NULL;

void cpu_set_gsbase(cpu_t *cpu)
{
    uint64_t addr = (uint64_t)cpu;
    trace_debug(MODULE, "setting GS base to %p", addr);
    msr_set_64(MSR_KERNEL_GS_BASE, addr);
    msr_set_64(MSR_GS_BASE, addr);
}

extern void hexdump(void *ptr, size_t len, size_t size);

void cpu_arch_init_bsp()
{
    ENTER_FUNC("", 0);
    cpu_t *cpu = &cpus[0];
    cpu->self = cpu;
    cpu->online = true;
    cpu->kernel_stack = kstack_per_cpu_alloc(NULL);
    bsp_cpu = cpu;

    hexdump(cpu->gdt_table, sizeof(cpu->gdt_table), 16);
    hexdump(&cpu->gdtr, sizeof(cpu->gdtr), 16);

    cpu_set_gsbase(cpu);

    lapic_timer_init(cpu->arch_id, cpu->logical_id);
    trace_info(MODULE, "local APIC enabled");

    trace_info("CPU", "BSP cpu_t init, APIC ID %u", cpu->arch_id);
}

cpu_t *cpu_get_current_indexed(leaf_0x1_0_t *regs_leaf1)
{
    uint32_t apic_id = 0;
    if (regs_leaf1->x2apic)
    {
        cpuid_regs regs_leaf0b;
        cpuid(0x0B, 0, &regs_leaf0b);
        apic_id = regs_leaf0b.edx;
        trace_info(MODULE, "x2apic = %u", apic_id);
    }
    else
    {
        apic_id = regs_leaf1->local_apic_id;
        trace_info(MODULE, "xapic = %u", apic_id);
    }
    return cpu_arch_get(apic_id);
}

void cpu_basic_setup(cpu_t *cpu)
{
    cpuid_regs regs_leaf;
    cpuid(0x00, 0, &regs_leaf);
    cpu->cpuid.leaf_0x0_0[0] = *((leaf_0x0_0_t *)((void *)&regs_leaf));

    cpuid(0x01, 0, &regs_leaf);
    cpu->cpuid.leaf_0x1_0[0] = *((leaf_0x1_0_t *)((void *)&regs_leaf));

    cpuid(0x07, 0, &regs_leaf);
    cpu->cpuid.leaf_0x7_0[0] = *((leaf_0x7_0_t *)((void *)&regs_leaf));

    cpuid(0x07, 1, &regs_leaf);
    cpu->cpuid.leaf_0x7_1[0] = *((leaf_0x7_1_t *)((void *)&regs_leaf));

    gdt_initialize(&cpu->gdtr, cpu->gdt_table);
    tss_initialize(&cpu->tss, cpu->gdt_table, TSS_INDEX);

    gdt_load(&cpu->gdtr, cpu->gdt_table);

    tss_load(TSS_SELECTOR);

    idt_load();
}

status_t cpu_init_cpu()
{
    cpuid_regs regs_leaf;
    cpuid(0x01, 0, &regs_leaf);

    cpu_t *cpu = cpu_get_current_indexed((leaf_0x1_0_t *)((void *)&regs_leaf));
    trace_info(MODULE, "cpu = %p", cpu);

    cpu_basic_setup(cpu);

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

    return KERRNO_SUCCESSES;
}

ARCH_INITCALL(cpu_init_cpu);

status_t cpu_init_ap(uint32_t apic_id, cpu_t *cpu)
{
    cpu_basic_setup(cpu);

    cpu->kernel_stack = kstack_per_cpu_alloc(NULL);

    mmu_arch_load_table(&kernel_page);

    cpu->self = cpu;
    cpu_set_gsbase(cpu);

    cpu->online = true;
    trace_info(MODULE, "AP %u init done", apic_id);
    return KERRNO_SUCCESSES;
}

status_t cpu_create(cpu_entry_t *entry)
{
    cpu_t *cpu = &cpus[entry->logical_id];
    if (!entry->is_bsp)
    {
        memset(cpu, 0, sizeof(cpu_t));
    }
    cpu->irq_handlers[EXC_SYSCALL - IRQ_BASE].state = HANDLER_IN_USE;
    cpu->arch_id = entry->arch_id;
    cpu->logical_id = entry->logical_id;
    entry->cpu = cpu;
    if (entry->is_bsp)
    {
        bsp_cpu = cpu;
    }
    return KERRNO_SUCCESSES;
}

void cpu_registered(cpu_logical_id_t id)
{
    if (id < CONFIG_MAX_CPUS)
    {
        cpus[id].irq_count++;
    }
}

cpu_t *cpu_arch_get_bsp()
{
    return bsp_cpu;
}

cpu_t *cpu_arch_get_current()
{
    return (cpu_t *)msr_get_64(MSR_GS_BASE);
}

cpu_t *cpu_arch_get(cpu_logical_id_t id)
{
    if (id < CONFIG_MAX_CPUS)
    {
        return &cpus[id];
    }
    return NULL;
}

void cpu_arch_set_kernel_stack(cpu_t *cpu, vaddr_t stack_top)
{
    cpu->tss.sp0 = stack_top;
}
