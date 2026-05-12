/*
 * File: MADT.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "MADT.h"
#include "kernel.h"
#include "debug/debug.h"

#define MODULE "MADT"

void MADT_parse_madt(apic_priv_t *priv)
{
    log_debug(MODULE, "got to here");
    MADT *madt = (MADT *)ACPI_get_table("APIC");
    if (!madt)
    {
        log_crit(MODULE, "no MADT");
        return;
    }

    priv->local_apic_base = madt->local_apic_address;

    uint8_t *entry = madt->entries;
    uint8_t *end = (uint8_t *)madt + madt->header.length;

    while (entry < end)
    {
        MADT_entry *en = (MADT_entry *)entry;
        switch (en->type)
        {
        case 0: // local APIC
        {
            MADT_local_apic *la = (MADT_local_apic *)&en->local;
            log_info(MODULE, "CPU %u APIC ID %u flags=%u",
                     la->processor_id, la->apic_id, la->flags);

            // bit 0 = processor enabled, bit 1 = online capable
            if ((la->flags & 1) || (la->flags & 2))
            {
                if (priv->cpu_count < MAX_CPUS)
                {
                    priv->cpus[priv->cpu_count].processor_id = la->processor_id;
                    priv->cpus[priv->cpu_count].apic_id = la->apic_id;
                    priv->cpus[priv->cpu_count].flags = la->flags;
                    priv->cpu_count++;
                }
            }
            break;
        }
        case 1: // I/O APIC
        {
            MADT_io_apic *ia = (MADT_io_apic *)&en->io;
            priv->io_apic_base = ia->io_apic_address;
            log_info(MODULE, "IOAPIC ID %u base=0x%x GSI base=%u",
                     ia->io_apic_id, ia->io_apic_address,
                     ia->global_system_interrupt_base);
            break;
        }
        case 2: // interrupt source override
        {
            MADT_iso *iso = (MADT_iso *)&en->iso;
            log_info(MODULE, "ISO IRQ %u -> GSI %u flags=0x%x",
                     iso->source, iso->global_system_interrupt, iso->flags);

            if (priv->iso_count < 16)
            {
                priv->isos[priv->iso_count].source = iso->source;
                priv->isos[priv->iso_count].gsi = iso->global_system_interrupt;
                priv->isos[priv->iso_count].flags = iso->flags;
                priv->iso_count++;
            }
            break;
        }
        }
        entry += en->length; // advance by entry length, not sizeof
    }

    log_info(MODULE, "local APIC base=0x%x IOAPIC base=0x%x",
             priv->local_apic_base, priv->io_apic_base);
}