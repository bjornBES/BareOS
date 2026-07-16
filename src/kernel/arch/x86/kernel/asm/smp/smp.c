/*
 * File: smp.c
 * File Created: 08 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/smp.h"
#include "cpu.h"
#include "kernel/asm/acpi/apic/lapic.h"
#include "kernel/asm/cpuid/cpuid.h"
#include "kernel/irq.h"
#include "kernel/ivt.h"

#include "mm/mmu/mmu.h"
#include "mm/kstack/kstack_allocator.h"
#include "task/threading/scheduling/scheduler.h"
#include "kernel/asm/segment/gdt.h"
#include "kernel/asm/ivt/idt.h"
#include "kernel/asm/syscall/syscall.h"
#include "debug/debug.h"
#include "time/timer.h"

#define MODULE "SMP"

extern char __trampoline_start;
extern char __trampoline_end;
paddr_t trampoline;
void *lapic_base;
int total_cores_init = 0;

void ap_startup(uint64_t apic_id)
{
    log_info("CPU", "AP %u started", apic_id);
    irq_arch_disable();
    cpu_init_ap(apic_id);
    ivt_arch_init();
    lapic_enable();
    lapic_timer_init();
    syscall_per_cpu_init();
    irq_arch_enable();
    log_info("CPU", "AP %u online", apic_id);

    total_cores_init++;
    uint64_t tick = 0;
    while (true)
    {
        __asm__ volatile("hlt");
        tick++;
        if (tick % 100 == 0)
        {
            // log_debug("CPU", "AP %u heartbeat tick=%u", apic_id, tick);
        }
    }
}

static smp_cpu_entry_t cpu_entries[MAX_CPUS];
static uint32_t cpu_count = 0;

void smp_arch_register_cpu(madt_local_apic_t *entry)
{
    if (!(entry->flags & 1) || (entry->flags & 2))
    {
        return; // skip disabled CPUs
    }

    cpu_entries[cpu_count].apic_id = entry->apic_id;
    cpu_entries[cpu_count].processor_id = entry->processor_id;
    cpu_entries[cpu_count].enabled = true;
    cpu_count++;
}

extern void hexdump(void *ptr, int len);

void smp_arch_boot_ap(smp_cpu_entry_t *entry)
{
    vaddr_t ap_stack = kstack_alloc(); // allocate kernel stack for this AP

    uint64_t *orig = (uint64_t *)(trampoline + 2);
    orig[0] = (uint64_t)kernel_page.page_dir_phys;
    orig[1] = (uint64_t)ap_stack; // stack top for this AP
    orig[2] = (uint64_t)&gdt_descriptor;
    orig[3] = (uint64_t)ap_startup;
    log_debug(MODULE, "sat up data for ap");

    log_debug(MODULE, "here0");
    // target AP in ICR high (bits 24-31 = destination APIC ID)
    lapic_write(LAPIC_REG_ICR_HIGH, (uint32_t)entry->apic_id << 24);
    log_debug(MODULE, "here1");

    // send INIT IPI
    // delivery mode 101 (INIT), level assert, edge triggered
    lapic_write(LAPIC_REG_ICR_LOW, 0x00004500);
    lapic_wait_idle();
    for (size_t i = 0; i < 10000; i++)
    {
    }
    log_debug(MODULE, "here2");

    // send first SIPI
    // delivery mode 110 (SIPI), vector = 0x09 (0x9000 >> 12)
    lapic_write(LAPIC_REG_ICR_HIGH, (uint32_t)entry->apic_id << 24);
    lapic_write(LAPIC_REG_ICR_LOW, 0x00004600 | 0x09);
    lapic_wait_idle();
    for (size_t i = 0; i < 200 * 10000; i++)
    {
    }
    log_debug(MODULE, "here3");

    // send second SIPI (spec says send twice)
    lapic_write(LAPIC_REG_ICR_HIGH, (uint32_t)entry->apic_id << 24);
    lapic_write(LAPIC_REG_ICR_LOW, 0x00004600 | 0x09);
    lapic_wait_idle();
    for (size_t i = 0; i < 200 * 10000; i++)
    {
    }
    log_debug(MODULE, "here4");
}

void smp_arch_init(boot_params_t *bp)
{
    irq_arch_disable();
    trampoline = (paddr_t)0x9000;

    // map 0x9000 identity mapped
    mmu_arch_map(&kernel_page, trampoline, trampoline, trampoline_flags);

    log_debug(MODULE, "tram=%p bp->smp_trampoline=%p", trampoline, bp->cpu_core_trampoline);

    lapic_base = (void *)priv->local_apic_base;

    lapic_id bspid = lapic_get_id(LAPIC_REG_ID);
    log_info(MODULE, "BSP APIC ID %u, starting %u APs", bspid, cpu_count - 1);

    int last_total_cores_init = 0;
    for (size_t i = 0; i < cpu_count; i++)
    {
        last_total_cores_init = total_cores_init;
        if (cpu_entries[i].apic_id == bspid)
        {
            cpu_entries[i].is_bsp = true;
            continue;
        }
        smp_arch_boot_ap(&cpu_entries[i]);
        while (last_total_cores_init == total_cores_init)
        {
            ;
        }
    }

    log_debug(MODULE, "sleep");
    for (size_t i = 0; i < 200 * 10000; i++)
    {
    }
    log_debug(MODULE, "done");

    cpu_init_bsp();
    irq_arch_enable();
}
