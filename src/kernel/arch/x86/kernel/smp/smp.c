/*
 * File: smp.c
 * File Created: 08 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "smp.h"
#include "cpu.h"
#include "kernel/apic/lapic.h"

#include "memory/paging/paging.h"
#include "memory/kstack/kstack_allocator.h"
#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "debug/debug.h"
#include "time/timer.h"

#define MODULE "SMP"

extern char __trampoline_start;
extern char __trampoline_end;
phys_addr trampoline;
void *lapic_base;
int total_cores_init = 0;

void ap_startup(uint64_t apic_id)
{
    log_info("CPU", "AP %u started", apic_id);
    cpu_init_ap(apic_id);
    x86_idt_load(kernel_idtr, kernel_idt_table);
    lapic_enable();
    lapic_timer_init();
    enable_interrupts();
    log_info("CPU", "AP %u online", apic_id);

    total_cores_init++;
    uint64_t tick = 0;
    while (true)
    {
        __asm__ volatile("hlt");
        tick++;
        if (tick % 100 == 0)
            log_debug("CPU", "AP %u heartbeat tick=%u", apic_id, tick);
    }
}
extern void hexdump(void *ptr, int len);

void smp_start_ap(uint8_t apic_id)
{
    void *ap_stack = kstack_alloc(); // allocate kernel stack for this AP

    uint64_t *orig = (uint64_t *)(trampoline + 2);
    orig[0] = (uint64_t)kernel_page.page_dir_phys;
    orig[1] = (uint64_t)ap_stack; // stack top for this AP
    orig[2] = (uint64_t)&gdt_descriptor;
    orig[3] = (uint64_t)ap_startup;

    // target AP in ICR high (bits 24-31 = destination APIC ID)
    lapic_write(LAPIC_REG_ICR_HIGH, (uint32_t)apic_id << 24);

    // send INIT IPI
    // delivery mode 101 (INIT), level assert, edge triggered
    lapic_write(LAPIC_REG_ICR_LOW, 0x00004500);
    lapic_wait_idle();
    timer_sleep_ms(10);

    // send first SIPI
    // delivery mode 110 (SIPI), vector = 0x09 (0x9000 >> 12)
    lapic_write(LAPIC_REG_ICR_HIGH, (uint32_t)apic_id << 24);
    lapic_write(LAPIC_REG_ICR_LOW, 0x00004600 | 0x09);
    lapic_wait_idle();
    timer_sleep_ms(1);

    // send second SIPI (spec says send twice)
    lapic_write(LAPIC_REG_ICR_HIGH, (uint32_t)apic_id << 24);
    lapic_write(LAPIC_REG_ICR_LOW, 0x00004600 | 0x09);
    lapic_wait_idle();
    timer_sleep_ms(10);
}

void smp_init(boot_params *bp)
{
    disableInterrupts();
    size_t size = 512;
    trampoline = (phys_addr)0x9000;

    // map 0x9000 identity mapped
    paging_map_page(kernel_page, trampoline, trampoline, trampoline_flags);

    log_debug(MODULE, "tram=%p bp->smp_trampoline=%p", trampoline, bp->smp_trampoline);

    lapic_base = (void *)priv->local_apic_base;

    uint8_t bspid = (uint8_t)(*((volatile uint32_t *)((uint8_t *)lapic_base + 0x20)) >> 24);
    log_info(MODULE, "BSP APIC ID %u, starting %u APs", bspid, priv->cpu_count - 1);

    int last_total_cores_init = 0;
    for (size_t i = 0; i < priv->cpu_count; i++)
    {
        last_total_cores_init = total_cores_init;
        if (priv->cpus[i].apic_id == bspid)
        {
            continue;
        }
        smp_start_ap(priv->cpus[i].apic_id);
        while (last_total_cores_init == total_cores_init)
        {
            ;
        }
    }

    log_debug(MODULE, "sleep");
    timer_sleep_ms(1000);
    log_debug(MODULE, "done");

    cpu_init_bsp();
    enable_interrupts();
}
