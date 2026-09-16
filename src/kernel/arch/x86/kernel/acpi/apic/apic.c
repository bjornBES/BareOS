/*
 * File: apic.c
 * File Created: 15 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/acpi/apic/apic.h"
#include "kernel/acpi/apic/ioapic.h"
#include "kernel/acpi/apic/lapic.h"

#include "cpu/cpu.h"

#include "debug/debug.h"

#include "irq/irq.h"

#define MODULE "x86-apic"

status_t apic_probe()
{
    if (local_apic_base == 0)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "madt has not been initialize and parsed");
    }

    if (ioapic_count == 0)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "madt has not been initialize and parsed");
    }

    return KERRNO_SUCCESSES;
}

status_t apic_initialize()
{
    // enable local APIC, set spurious vector to 0xFF
    lapic_enable();

    for (uint8_t i = 0; i < ioapic_count; i++)
    {
        ioapic_entry_t *entry = ioapic_get_entry(i);
        log_info(MODULE, "entry = %p", entry);
        log_debug(NO_MODULE, "IOAPIC %u = {base: %p, gsi range: %u-%u, redir limit: %u}", i, entry->io_apic_base, entry->gsi_base, entry->gsi_end, entry->ioapic_max_redir);

        for (uint8_t irq = entry->gsi_base; irq < entry->ioapic_max_redir + 1; irq++)
        {
            gsi_t gsi = entry->redir_entries[irq].gsi;
            uint16_t flags = entry->redir_entries[irq].flags;
            uint8_t vector = entry->redir_entries[irq].vector;
            ioapic_set_entry(i, gsi, vector, flags, entry->redir_entries[irq].lapic_target);
            log_info(MODULE, "IRQ %u -> GSI %u vector 0x%x", irq, gsi, vector);
        }
    }
    return KERRNO_SUCCESSES;
}

int apic_route(gsi_t gsi, interrupt_vector_t vector, cpu_logical_id_t target, irq_trigger_t trigger, irq_polarity_t polarity)
{
    ENTER_FUNC("%u, %u, %u, %u, %u", gsi, vector, target, trigger, polarity);
    cpu_entry_t *cpu = cpu_get_entry(target);
    uint8_t ioapic_id = ioapic_get_id(gsi);
    log_debug(MODULE, "ioapic_id = %u", ioapic_id);
    FUNC_NOT_IMPLEMENTED();
    return 0;
}

void apic_disable()
{
    FUNC_NOT_IMPLEMENTED();
}

void apic_send_eoi(gsi_t gsi)
{
    FUNC_NOT_IMPLEMENTED();
}

void apic_mask(gsi_t gsi)
{
    FUNC_NOT_IMPLEMENTED();
}

void apic_unmask(gsi_t gsi)
{
    FUNC_NOT_IMPLEMENTED();
}

bool apic_is_masked(gsi_t gsi)
{
    FUNC_NOT_IMPLEMENTED();
    return false;
}

static irq_controller_t apic_driver = {
    .name = "apic",
    .probe = apic_probe,
    .initialize = apic_initialize,
    .route = apic_route,
    .disable = apic_disable,
    .send_eoi = apic_send_eoi,
    .mask = apic_mask,
    .unmask = apic_unmask,
    .is_masked = apic_is_masked,
};

irq_controller_t *apic_get_driver()
{
    return &apic_driver;
}
