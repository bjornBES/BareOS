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

#include "kernel/cpu/cpu.h"
#include "kernel/cpuid/cpuid.h"
#include "kernel/msr/msr.h"
#include "kernel/acpi/apic/lapic.h"

#include "entry/desc/idt/idt.h"

#include "mm/allocator/kstack_allocator.h"

#include "debug/debug.h"

#include "memory.h"

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

status_t cpu_init_ap(uint32_t apic_id, cpu_t *cpu)
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

    cpu->kernel_stack = kstack_per_cpu_alloc(NULL);

    gdt_initialize(&cpu->gdtr, cpu->gdt_table);
    tss_initialize(&cpu->tss, cpu->gdt_table, TSS_INDEX);

    gdt_load(&cpu->gdtr, cpu->gdt_table);

    tss_load(TSS_SELECTOR);

    idt_load();

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
    cpu->arch_id = entry->arch_id;
    cpu->logical_id = entry->logical_id;
    entry->cpu = cpu;
    if (entry->is_bsp)
    {
        bsp_cpu = cpu;
    }
    return KERRNO_SUCCESSES;
}

cpu_t *cpu_arch_get_bsp()
{
    return &cpus[0];
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
