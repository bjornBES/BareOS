/*
 * File: cpu.c
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "cpu.h"
#include "processor_ctl.h"
#include "kernel.h"
#include "debug/debug.h"
#include "kernel/asm/MSR/MSR.h"
#include "kernel/asm/acpi/apic/lapic.h"

#include "mm/kstack/kstack_allocator.h"

cpu_t cpus[MAX_CPUS] = {0};
cpu_t *bsp_cpu = NULL;

// called once per CPU during init
void cpu_set_gs(cpu_t *cpu)
{
    uint64_t addr = (uint64_t)cpu;
    log_debug("CPU", "setting GS base to %p", addr);
    msr_set_64(MSR_KERNEL_GS_BASE, addr);
    msr_set_64(MSR_GS_BASE, addr);
    /*     __asm__ volatile(
            "wrmsr"
            :
            : "c"(0xC0000101), // IA32_GS_BASE MSR
              "a"((uint32_t)addr),
              "d"((uint32_t)(addr >> 32))); */
}

cpu_t *cpu_arch_get_current()
{
    // uint32_t lo, hi;
    return (cpu_t *)msr_get_64(MSR_GS_BASE);
    /* __asm__ volatile(
        "mov ecx, 0xC0000101\n"
        "rdmsr\n"
        : "=a"(lo), "=d"(hi)
        :
        : "ecx"
    );
    return (cpu_t *)(((uint64_t)hi << 32) | lo); */
}

cpu_t *cpu_get(cpu_id id)
{
    if (id < MAX_CPUS)
    {
        return &cpus[id];
    }
    return NULL;
}

void cpu_init_bsp()
{
    cpu_t *cpu = &cpus[0];
    cpu->self = cpu;
    cpu->apic_id = lapic_get_id();
    cpu->cpu_id = 0;
    cpu->online = true;
    cpu->current = NULL; // scheduler fills this
    cpu->kernel_stack = kstack_per_cpu_alloc();
    bsp_cpu = cpu;

    ctl_cr0_add(X86_CR0_MP);
    ctl_cr0_remove(X86_CR0_EM);

    ctl_cr4_add(X86_CR4_OSFXSR | X86_CR4_OSXMMEXCPT);

    cpu_set_gs(cpu);

    log_info("CPU", "BSP cpu_t init, APIC ID %u", cpu->apic_id);
}

void cpu_init_ap(lapic_id apic_id)
{
    // find free slot
    cpu_t *cpu = NULL;
    for (cpu_id i = 1; i < MAX_CPUS; i++) // 0 = BSP
    {
        if (!cpus[i].online)
        {
            cpu = &cpus[i];
            cpu->cpu_id = i;
            break;
        }
    }
    if (!cpu)
    {
        log_crit("CPU", "no free cpu_t for APIC %u", apic_id);
        return;
    }

    cpu->apic_id = apic_id;

    cpu->kernel_stack = kstack_per_cpu_alloc();

    // copy BSP GDT as base
    memcpy(cpu->gdt, gdt_table, sizeof(gdt_table));

    // set up GDTR for this CPU's GDT copy
    cpu->gdtr.Ptr = cpu->gdt;
    cpu->gdtr.limit = sizeof(cpu->gdt) - 1;

    // load the new GDT + TSS
    x86_GDT_load(&cpu->gdtr, cpu->gdt);

    // init this CPU's TSS — zero it, patch descriptor to point to our tss
    memset(&cpu->tss, 0, sizeof(tss_t));
    tss_initialize(&cpu->tss, &cpu->gdt[TSS_INDEX]);

    x86_GDT_dump_selector(TSS_SELECTOR);

    tss_load(TSS_SELECTOR);

    ctl_cr0_add(X86_CR0_MP);
    ctl_cr0_remove(X86_CR0_EM);

    ctl_cr4_add(X86_CR4_OSFXSR | X86_CR4_OSXMMEXCPT);

    cpu->self = cpu;
    cpu_set_gs(cpu);

    cpu->online = true;
    log_info("CPU", "AP %u init done", apic_id);
}
