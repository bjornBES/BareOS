/*
 * File: ioapic.c
 * File Created: 19 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/acpi/apic/ioapic.h"

#include "asm/irq_arch.h"

#include "mm/ioremap.h"

#include "resource/resource.h"

#include "kerrno.h"

#include "string.h"

#include <binary.h>

#define MODULE "x86-ioapic"

uint8_t ioapic_count = 0;
ioapic_entry_t ioapic_table[16];

void ioapic_write(uint8_t ioapic_id, uint32_t reg, uint32_t value)
{
    ioapic_entry_t *entry = &ioapic_table[ioapic_id];
    *(volatile uint32_t *)(entry->io_apic_base + IOAPIC_REG_SELECT) = reg;
    *(volatile uint32_t *)(entry->io_apic_base + IOAPIC_REG_WINDOW) = value;
}

uint32_t ioapic_read(uint8_t ioapic_id, uint32_t reg)
{
    ioapic_entry_t *entry = &ioapic_table[ioapic_id];
    *(volatile uint32_t *)(entry->io_apic_base + IOAPIC_REG_SELECT) = reg;
    return *(volatile uint32_t *)(entry->io_apic_base + IOAPIC_REG_WINDOW);
}

status_t ioapic_print_entry(uint8_t ioapic_id, gsi_t gsi)
{
    ioapic_entry_t *entry = &ioapic_table[ioapic_id];
    iso_entry_t *iso_entry = &entry->redir_entries[gsi - entry->gsi_base];
    trace_info(MODULE, "entry @ %p index %u = {vector: 0x%x, gsi: %u, irq: %u, pin: %u, flags: 0x%x, lapic: %u, locked: %s, dont use: %s, trigger: %u, polarity: %u}",
               iso_entry, gsi - entry->gsi_base,
               iso_entry->vector, iso_entry->gsi, iso_entry->irq, iso_entry->pin, iso_entry->flags, iso_entry->lapic_target, iso_entry->locked BOOL_TO_STRING, iso_entry->dont_use BOOL_TO_STRING, iso_entry->trigger, iso_entry->polarity);
    return KERRNO_SUCCESSES;
}

status_t ioapic_register(uint8_t ioapic_id, paddr_t ioapic_address, uint32_t gsi_base)
{
    resource_t *ioapic = resource_create();
    resource_request(&mem_space.root, ioapic, ioapic_address, ioapic_address + 4096, "IOAPIC", RES_TYPE_PLATFORM_FIXED, RES_FLAG_NONE);

    ioapic_table[ioapic_count].ioapic_id = ioapic_id;
    ioapic_table[ioapic_count].io_apic_base = ioremap(ioapic_address, 4096);
    ioapic_table[ioapic_count].gsi_base = gsi_base;
    uint32_t version = ioapic_read(ioapic_id, IOAPIC_REG_VERSION);
    ioapic_table[ioapic_count].ioapic_max_redir = BIT_GET_RANGE(version, 16, 23);
    ioapic_table[ioapic_count].gsi_end = gsi_base + ioapic_table[ioapic_id].ioapic_max_redir;
    trace_debug(MODULE, "IOAPIC %u = {base: %p, gsi range: %u-%u, redir limit: %u}", ioapic_count, ioapic_table[ioapic_count].io_apic_base, ioapic_table[ioapic_count].gsi_base, ioapic_table[ioapic_count].gsi_end, ioapic_table[ioapic_count].ioapic_max_redir);
    ioapic_table[ioapic_id].redir_entries[2].dont_use = true;
    ioapic_entry_t *entry = &ioapic_table[ioapic_count];
    for (uint32_t i = gsi_base; i < entry->ioapic_max_redir + 1; i++)
    {
        iso_entry_t *iso_entry = &entry->redir_entries[i - gsi_base];
        iso_entry->flags = 0b0101;
        iso_entry->trigger = IRQ_TRIGGER_EDGE;
        iso_entry->polarity = IRQ_POLARITY_HIGH;
        iso_entry->irq = KERNEL_IRQ_NONE;
        iso_entry->gsi = i;
        iso_entry->vector = i + IRQ_BASE;
        iso_entry->lapic_target = 0;
        iso_entry->pin = i;
        ioapic_print_entry(ioapic_count, i);
    }
    ioapic_count++;

    return KERRNO_SUCCESSES;
}

ioapic_entry_t *ioapic_get_entry(uint8_t ioapic_id)
{
    return &ioapic_table[ioapic_id];
}

uint8_t ioapic_get_id(gsi_t target_gsi)
{
    for (uint8_t i = 0; i < ioapic_count; i++)
    {
        ioapic_entry_t *entry = &ioapic_table[i];
        trace_debug(MODULE, "IOAPIC %u: %u within %u-%u}", i, target_gsi, entry->gsi_base, entry->gsi_end);
        if (target_gsi < entry->gsi_base)
        {
            continue;
        }
        if (target_gsi > entry->gsi_end)
        {
            continue;
        }
        return i;
    }
    return 0xFF;
}

void ioapic_set_overwrite(uint8_t ioapic_id, gsi_t gsi, uint8_t irq, uint16_t flags, uint32_t dest_apic_id)
{
    ioapic_entry_t *entry = &ioapic_table[ioapic_id];
    iso_entry_t *iso_entry = &entry->redir_entries[gsi - entry->gsi_base];
    iso_entry->flags = flags;
    iso_entry->gsi = gsi;
    iso_entry->lapic_target = dest_apic_id;
    iso_entry->vector = irq + IRQ_BASE;
    iso_entry->pin = irq;
    iso_entry->locked = true;
    iso_entry->trigger = (flags & 0xC) == 0b1100;
    iso_entry->polarity = (flags & 0x3) == 0b11;

    ioapic_set_entry(ioapic_id, gsi, irq, iso_entry->trigger, iso_entry->polarity, dest_apic_id);
}

void ioapic_set_entry(uint8_t ioapic_id, gsi_t gsi, uint8_t irq, irq_trigger_t trigger, irq_polarity_t polarity, uint32_t dest_apic_id)
{
    // ENTER_FUNC("%u, %u, %u, 0x%x, 0x%x", ioapic_id, gsi, vector, flags, dest_apic_id);
    uint32_t low = (irq + IRQ_BASE);

    uint16_t flags = 0;
    if (polarity == IRQ_POLARITY_HIGH)
    {
        flags |= 0b01;
    }
    else
    {
        // polarity — bit 1 of flags, 1 = active low
        low |= (1 << 13);
        flags |= 0b11;
    }

    if (trigger == IRQ_TRIGGER_EDGE)
    {
        flags |= 0b0100;
    }
    else
    {
        // trigger mode — bit 3 of flags, 1 = level triggered
        low |= (1 << 15);
        flags |= 0b1100;
    }

    ioapic_entry_t *entry = &ioapic_table[ioapic_id];
    iso_entry_t *iso_entry = &entry->redir_entries[gsi - entry->gsi_base];
    iso_entry->flags = flags;
    iso_entry->gsi = gsi;
    iso_entry->lapic_target = dest_apic_id;
    iso_entry->vector = irq + IRQ_BASE;
    // iso_entry->pin = irq;

    // start masked
    low |= (1 << 16);

    // destination = local APIC ID , uint32_t dest_apic_id
    uint32_t high = (dest_apic_id << 24);

    ioapic_write(ioapic_id, IOAPIC_REG_REDTBL + gsi * 2, low);
    ioapic_write(ioapic_id, IOAPIC_REG_REDTBL + gsi * 2 + 1, high);
}

void ioapic_set_apic_dest(uint8_t ioapic_id, gsi_t gsi, uint32_t dest_apic_id)
{
    // destination = local APIC ID , uint8_t dest_apic_id
    uint32_t high = (dest_apic_id << 24);
    ioapic_write(ioapic_id, IOAPIC_REG_REDTBL + gsi * 2 + 1, high);
}

void ioapic_unmask(uint8_t ioapic_id, uint32_t irq)
{
    uint32_t low = ioapic_read(ioapic_id, IOAPIC_REG_REDTBL + irq * 2);
    ioapic_write(ioapic_id, IOAPIC_REG_REDTBL + irq * 2, low & ~(1 << 16));
}

void ioapic_mask(uint8_t ioapic_id, uint32_t irq)
{
    uint32_t low = ioapic_read(ioapic_id, IOAPIC_REG_REDTBL + irq * 2);
    ioapic_write(ioapic_id, IOAPIC_REG_REDTBL + irq * 2, low | (1 << 16));
}

uint32_t ioapic_is_mask(uint8_t ioapic_id, uint32_t irq)
{
    uint32_t low = ioapic_read(ioapic_id, IOAPIC_REG_REDTBL + irq * 2);
    return (low >> 16) & 1;
}
