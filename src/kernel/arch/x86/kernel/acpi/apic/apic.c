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

#include "string.h"

#define MODULE "x86-apic"

bool enabled_x2apic = false;

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
        trace_debug(MODULE, "entry = %p", entry);
        trace_info(MODULE, "IOAPIC %u = {base: %p, gsi range: %u-%u, redir limit: %u}", i, entry->io_apic_base, entry->gsi_base, entry->gsi_end, entry->ioapic_max_redir);

        for (uint8_t irq_index = entry->gsi_base; irq_index < entry->ioapic_max_redir + 1; irq_index++)
        {
            iso_entry_t *iso_entry = &entry->redir_entries[irq_index - entry->gsi_base];
            gsi_t gsi = iso_entry->gsi;
            uint16_t flags = iso_entry->flags;
            uint8_t vector = iso_entry->vector;
            ioapic_set_entry(entry->ioapic_id, gsi, vector - IRQ_BASE, iso_entry->trigger, iso_entry->polarity, iso_entry->lapic_target);
            // trace_info(MODULE, "IRQ %u -> GSI %u vector 0x%x", iso_entry->pin, gsi, vector);
            ioapic_print_entry(i, irq_index);
        }
    }
    lapic_eoi();
    return KERRNO_SUCCESSES;
}

gsi_t apic_pick_free_gsi(uint32_t allowed_pins)
{
    for (uint8_t i = 0; i < ioapic_count; i++)
    {
        ioapic_entry_t *entry = ioapic_get_entry(i);
        trace_debug(MODULE, "entry = %p", entry);
        trace_info(MODULE, "IOAPIC %u = {base: %p, gsi range: %u-%u, redir limit: %u}", i, entry->io_apic_base, entry->gsi_base, entry->gsi_end, entry->ioapic_max_redir);

        for (uint8_t irq_index = entry->gsi_base; irq_index < entry->ioapic_max_redir + 1; irq_index++)
        {
            iso_entry_t *iso_entry = &entry->redir_entries[irq_index - entry->gsi_base];
            // ioapic_print_entry(i, irq_index);
            if (!(allowed_pins & (1ull << irq_index)))
            {
                continue;
            }

            if (iso_entry->dont_use)
            {
                continue;
            }

            if (iso_entry->irq != KERNEL_IRQ_NONE)
            {
                continue;
            }

            return irq_index;
        }
    }
    return ~0;
}

gsi_t apic_irq_to_gsi(kernel_irq_t irq)
{
    ENTER_FUNC("%u", irq);
    gsi_t gsi = (~0);
    for (uint8_t i = 0; i < ioapic_count; i++)
    {
        ioapic_entry_t *entry = ioapic_get_entry(i);

        for (uint8_t irq_index = entry->gsi_base; irq_index < entry->ioapic_max_redir + 1; irq_index++)
        {
            iso_entry_t *iso_entry = &entry->redir_entries[irq_index - entry->gsi_base];
            if (iso_entry->irq == irq)
            {
                return iso_entry->gsi;
            }
        }
    }
    return KERRNO_UNSUCCESS;
}

kernel_irq_t apic_lookup_gsi(gsi_t gsi, irq_trigger_t *trigger, irq_polarity_t *polarity)
{
    ioapic_entry_t *entry = ioapic_get_entry(ioapic_get_id(gsi));
    iso_entry_t *iso_entry = &entry->redir_entries[gsi - entry->gsi_base];
    *trigger = iso_entry->trigger;
    *polarity = iso_entry->polarity;
    return iso_entry->irq;
}

status_t apic_route(kernel_irq_t irq, interrupt_vector_t vector, cpu_logical_id_t target, irq_trigger_t trigger, irq_polarity_t polarity)
{
    ENTER_FUNC("%u, 0x%x, %u, %u, %u", irq, vector, target, trigger, polarity);
    cpu_entry_t *cpu = cpu_get_entry(target);
    gsi_t gsi = (~0);
    uint8_t ioapic_id = 0;
    for (uint8_t i = 0; i < ioapic_count; i++)
    {
        ioapic_entry_t *entry = ioapic_get_entry(i);
        // trace_debug(MODULE, "entry = %p", entry);
        // trace_info(MODULE, "IOAPIC %u = {base: %p, gsi range: %u-%u, redir limit: %u}", i, entry->io_apic_base, entry->gsi_base, entry->gsi_end, entry->ioapic_max_redir);

        for (uint8_t irq_index = entry->gsi_base; irq_index < entry->ioapic_max_redir + 1; irq_index++)
        {
            // ioapic_print_entry(ioapic_id, irq_index);
            iso_entry_t *iso_entry = &entry->redir_entries[irq_index - entry->gsi_base];
            if (iso_entry->irq == irq)
            {
                ioapic_id = i;
                gsi = iso_entry->gsi;
                break;
            }
        }

        if (gsi != (~0))
        {
            break;
        }
    }
    ioapic_print_entry(ioapic_id, gsi);
    ioapic_entry_t *entry = ioapic_get_entry(ioapic_id);
    iso_entry_t *iso_entry = &entry->redir_entries[gsi - entry->gsi_base];
    if (iso_entry->dont_use)
    {
        return KERRNO_UNSUCCESS;
    }
    if (iso_entry->locked)
    {
        if (iso_entry->vector != vector || iso_entry->pin != vector - IRQ_BASE)
        {
            trace_debug(MODULE, "vector: 0x%x == 0x%x, gsi: %u == %u, pin: %u == %u",
                        iso_entry->vector, vector, iso_entry->gsi, gsi, iso_entry->pin, vector - IRQ_BASE);
            return KERRNO_UNSUCCESS;
        }
    }
    iso_entry->irq = irq;
    iso_entry->vector = vector;
    iso_entry->lapic_target = cpu->arch_id;
    ioapic_set_entry(entry->ioapic_id, gsi, vector - IRQ_BASE, trigger, polarity, cpu->arch_id);
    trace_debug(MODULE, "ioapic_id = %u", ioapic_id);
    // FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESSES;
}

status_t apic_bind_gsi(gsi_t gsi, kernel_irq_t irq)
{
    ioapic_entry_t *entry = ioapic_get_entry(ioapic_get_id(gsi));
    iso_entry_t *iso_entry = &entry->redir_entries[gsi - entry->gsi_base];
    iso_entry->irq = irq;
    return KERRNO_SUCCESSES;
}

void apic_disable()
{
    FUNC_NOT_IMPLEMENTED();
}

void apic_send_eoi(gsi_t gsi)
{
    lapic_eoi();
}

void apic_mask(gsi_t gsi)
{
    ioapic_mask(ioapic_get_id(gsi), gsi);
}

void apic_unmask(gsi_t gsi)
{
    ioapic_unmask(ioapic_get_id(gsi), gsi);
}

bool apic_is_masked(gsi_t gsi)
{
    return ioapic_is_mask(ioapic_get_id(gsi), gsi);
}

status_t apic_debug_print()
{
    ENTER_FUNC("", 0);
    for (uint8_t i = 0; i < ioapic_count; i++)
    {
        ioapic_entry_t *entry = ioapic_get_entry(i);
        trace_info(MODULE, "IOAPIC %u = {base: %p, gsi range: %u-%u, redir limit: %u}", i, entry->io_apic_base, entry->gsi_base, entry->gsi_end, entry->ioapic_max_redir);

        for (uint8_t irq_index = entry->gsi_base; irq_index < entry->ioapic_max_redir + 1; irq_index++)
        {
            iso_entry_t *iso_entry = &entry->redir_entries[irq_index - entry->gsi_base];
            gsi_t gsi = iso_entry->gsi;
            uint16_t flags = iso_entry->flags;
            uint8_t vector = iso_entry->vector;
            ioapic_print_entry(i, irq_index);
            // trace_info(MODULE, "IRQ %u -> GSI %u vector 0x%x", iso_entry->pin, gsi, vector);
        }
    }
    return KERRNO_SUCCESSES;
}

static irq_controller_t apic_driver = {
    .name = "apic",
    .probe = apic_probe,
    .initialize = apic_initialize,

    .route = apic_route,

    .irq_to_gsi = apic_irq_to_gsi,
    .gsi_lookup = apic_lookup_gsi,

    .bind = apic_bind_gsi,

    .pick_free_pin = apic_pick_free_gsi,

    .disable = apic_disable,

    .mask = apic_mask,
    .unmask = apic_unmask,
    .is_masked = apic_is_masked,

    .send_eoi = apic_send_eoi,

    .debug = apic_debug_print,
};

irq_controller_t *apic_get_driver()
{
    return &apic_driver;
}
