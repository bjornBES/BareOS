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
    for (uint32_t i = gsi_base; i < ioapic_table[ioapic_count].ioapic_max_redir + 1; i++)
    {
        ioapic_table[ioapic_count].redir_entries[i].flags = 0;
        ioapic_table[ioapic_count].redir_entries[i].gsi = i;
        ioapic_table[ioapic_count].redir_entries[i].vector = i + IRQ_BASE;
        ioapic_table[ioapic_count].redir_entries[i].lapic_target = 0;
        ioapic_table[ioapic_count].redir_entries[i].pin = i;
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
    ioapic_table[ioapic_id].redir_entries[irq].flags = flags;
    ioapic_table[ioapic_id].redir_entries[irq].gsi = gsi;
    ioapic_table[ioapic_id].redir_entries[irq].lapic_target = dest_apic_id;
    ioapic_table[ioapic_id].redir_entries[irq].vector = irq + IRQ_BASE;
    ioapic_table[ioapic_id].redir_entries[irq].pin = irq;
    ioapic_table[ioapic_id].redir_entries[irq].locked = true;

    ioapic_set_entry(ioapic_id, gsi, irq, flags, dest_apic_id);
}

void ioapic_set_entry(uint8_t ioapic_id, gsi_t gsi, uint8_t irq, uint16_t flags, uint32_t dest_apic_id)
{
    // ENTER_FUNC("%u, %u, %u, 0x%x, 0x%x", ioapic_id, gsi, vector, flags, dest_apic_id);
    uint32_t low = (irq + IRQ_BASE);

    ioapic_table[ioapic_id].redir_entries[irq].flags = flags;
    ioapic_table[ioapic_id].redir_entries[irq].gsi = gsi;
    ioapic_table[ioapic_id].redir_entries[irq].lapic_target = dest_apic_id;
    ioapic_table[ioapic_id].redir_entries[irq].vector = irq + IRQ_BASE;
    ioapic_table[ioapic_id].redir_entries[irq].pin = irq;

    // polarity — bit 1 of flags, 1 = active low
    if (flags & 0x2)
    {
        low |= (1 << 13);
    }

    // trigger mode — bit 3 of flags, 1 = level triggered
    if (flags & 0x8)
    {
        low |= (1 << 15);
    }

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
