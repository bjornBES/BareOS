/*
 * File: madt.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/tables/madt.h"
#include "acpi/table.h"
#include "kernel/smp.h"
#include "kernel/irq.h"
#include "kernel.h"
#include "debug/debug.h"

#define MODULE "madt"

void madt_arch_parse(madt *madt)
{
    apic_set_local_base((paddr_t)madt->local_apic_address);

    uint8_t *entry = madt->entries;
    uint8_t *end = (uint8_t *)madt + madt->header.length;

    while (entry < end)
    {
        madt_entry *en = (madt_entry *)entry;
        switch (en->type)
        {
            case 0 : // local APIC
                {
                    madt_local_apic_t *la = (madt_local_apic_t *)&en->local;
                    log_info(MODULE, "CPU %u APIC ID %u flags=%u", la->processor_id, la->apic_id, la->flags);
                    smp_arch_register_cpu(la);
                    break;
                }
            case 1 : // I/O APIC
                {
                    madt_io_apic *ia = (madt_io_apic *)&en->io;
                    apic_set_io_base((paddr_t)ia->io_apic_address);
                    log_info(MODULE, "IOAPIC ID %u base=0x%x GSI base=%u",
                             ia->io_apic_id, ia->io_apic_address,
                             ia->global_system_interrupt_base);
                    break;
                }
            case 2 : // interrupt source override
                {
                    madt_iso *iso = (madt_iso *)&en->iso;
                    log_info(MODULE, "ISO IRQ %u -> GSI %u flags=0x%x",
                             iso->source, iso->global_system_interrupt, iso->flags);

                    irq_arch_register_override(iso->global_system_interrupt, iso->source, iso->flags);
                    break;
                }
        }
        entry += en->length; // advance by entry length, not sizeof
    }
}
