/*
 * File: smp.c
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/smp_arch.h"
#include "asm/mmu_arch.h"

#include "kernel/msr/msr.h"
#include "kernel/cpu/cpu.h"
#include "kernel/cpuid/cpuid.h"
#include "kernel/smp/ipi.h"
#include "kernel/acpi/apic/lapic.h"

#include "thread/thread.h"
#include "sched/sched.h"

#include "mm/allocator/kstack_allocator.h"

#include "debug/debug.h"

#include "memory.h"

#include <binary.h>

#define MODULE "x86-smp"

int total_cores_init = 0;

void ap_startup()
{
    inline_asm("cli" : : : "memory");
    cpuid_regs regs_leaf;
    uint32_t smt_shift = 0;
    uint32_t core_shift = 0;
    for (uint32_t subleaf = 0;; subleaf++)
    {
        cpuid(0x0B, subleaf, &regs_leaf);
        trace_info(MODULE, "0x0B.0x%02x = {eax = 0x%08x, ebx = 0x%08x, ecx = 0x%08x, edx = 0x%08x}", subleaf, regs_leaf.eax, regs_leaf.ebx, regs_leaf.ecx, regs_leaf.edx);
        uint32_t level_type = BIT_GET_RANGE(regs_leaf.ecx, 8, 15);
        
        if (level_type == 0)
        {
            break; // no more levels
        }
        if (level_type == 1)
        {
            smt_shift = regs_leaf.eax & 0x1F;
        }
        if (level_type == 2)
        {
            core_shift = regs_leaf.eax & 0x1F; // cumulative, includes SMT bits
        }
    }

    uint32_t apic_id = regs_leaf.edx; // from the last valid subleaf, this CPU's x2APIC id

    uint32_t smt_id = apic_id & ((1u << smt_shift) - 1);
    uint32_t core_id = (apic_id >> smt_shift) & ((1u << (core_shift - smt_shift)) - 1);
    uint32_t package_id = apic_id >> core_shift;
    trace_info(MODULE, "ap%u has = {smt = %u, core = %u, package id = %u}", apic_id, smt_id, core_id, package_id);

    cpu_t *cpu = cpu_arch_get(apic_id);
    trace_info("CPU", "AP %u started %p", apic_id, cpu);
    
    cpu_init_ap(apic_id, cpu);
    
    inline_asm("sti" : : : "memory");
    
    // mmu_arch_load_table(&kernel_page);
    
    lapic_enable();
    lapic_timer_init(apic_id, cpu->logical_id);

    trace_info("CPU", "AP %u online", apic_id);

    total_cores_init++;
    while (true)
    {
        inline_asm("sti" : : : "memory");
        inline_asm("hlt" : : : "memory");
    }
}

extern void hexdump(void *ptr, size_t len, size_t size);

void smp_arch_boot_ap(cpu_entry_t *entry, vaddr_t trampoline)
{
    int last_total_cores_init = total_cores_init;
    vaddr_t ap_stack = kstack_alloc(NULL); // allocate kernel stack for this AP

    uint64_t *orig = (uint64_t *)(trampoline + 2);
    orig[0] = (uint64_t)kernel_page.page_dir_phys;
    orig[1] = (uint64_t)ap_stack; // stack top for this AP
    orig[2] = (uint64_t)&entry->cpu->gdtr;
    orig[3] = (uint64_t)ap_startup;
    // hexdump((void *)trampoline, 64, 16);
    trace_debug(MODULE, "sat up data for ap%u", entry->arch_id);
    // trace_debug(MODULE, "rt var[0] = %p", kernel_page.page_dir_phys);
    // trace_debug(MODULE, "rt var[1] = %p", ap_stack);
    // trace_debug(MODULE, "rt var[2] = %p", &entry->cpu->gdtr);
    // trace_debug(MODULE, "rt var[3] = %p", ap_startup);

    // send INIT IPI
    // delivery mode 101 (INIT), level assert, edge triggered
    lapic_write_icr((uint32_t)entry->arch_id, MAKE_LOW(0, DELI_INIT, DEST_PHYS, DELI_STATUS_IDLE, LEVEL_ASSERT, TRIGGER_EDGE, DEST_NO));
    lapic_wait_idle();
    for (size_t i = 0; i < 10000; i++)
    {
    }
    trace_debug(MODULE, "here2");

    // send first SIPI
    // delivery mode 110 (SIPI), vector = 0x09 (0x9000 >> 12)
    lapic_write_icr((uint32_t)entry->arch_id, MAKE_LOW(0x09, DELI_STARTUP, DEST_PHYS, DELI_STATUS_IDLE, LEVEL_ASSERT, TRIGGER_EDGE, DEST_NO));
    lapic_wait_idle();
    for (size_t i = 0; i < 200 * 10000; i++)
    {
    }
    trace_debug(MODULE, "here3");

    // send second SIPI (spec says send twice)
    lapic_write_icr((uint32_t)entry->arch_id, MAKE_LOW(0x09, DELI_STARTUP, DEST_PHYS, DELI_STATUS_IDLE, LEVEL_ASSERT, TRIGGER_EDGE, DEST_NO));
    lapic_wait_idle();
    for (size_t i = 0; i < 200 * 10000; i++)
    {
    }
    trace_debug(MODULE, "here4");

    while (last_total_cores_init == total_cores_init)
    {
        ;
    }
}

void smp_arch_idle_thread()
{
    cpu_t *cpu = cpu_arch_get_current();
    trace_debug(MODULE, "cpu %u is going idle", cpu->arch_id);
    while (true)
    {
        __asm__ volatile("cli");
        if (sched_has_work(cpu) == KERRNO_SUCCESSES)
        {
            trace_debug(MODULE, "cpu %u found work", cpu->arch_id);
            __asm__ volatile("sti");
            sched_yield();
        }
        __asm__ volatile("sti");
        __asm__ volatile("hlt");
    }
}
THREAD_WARPER_NO_RETURN_ARG(smp_arch_idle_thread);

status_t smp_arch_send_ipi(uint32_t apic_id, interrupt_vector_t vector)
{
    ENTER_FUNC("%u, 0x%x", apic_id, vector);
    lapic_write_icr(apic_id, MAKE_LOW(vector, DELI_FIXED, DEST_PHYS, DELI_STATUS_IDLE, LEVEL_ASSERT, 0, DEST_NO));
    return KERRNO_SUCCESSES;
}

status_t smp_arch_send_ipi_all(interrupt_vector_t vector)
{
    lapic_write_icr(0xFFFFFFFF, MAKE_LOW(vector, DELI_FIXED, DEST_PHYS, DELI_STATUS_IDLE, LEVEL_ASSERT, 0, DEST_ALL));
    return KERRNO_SUCCESSES;
}

status_t smp_arch_send_ipi_others(interrupt_vector_t vector)
{
    lapic_write_icr(0xFFFFFFFF, MAKE_LOW(vector, DELI_FIXED, DEST_PHYS, DELI_STATUS_IDLE, LEVEL_ASSERT, 0, DEST_EXCLUDE_SELF));
    return KERRNO_SUCCESSES;
}

