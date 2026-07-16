/*
 * File: ioapic.c
 * File Created: 19 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ioapic.h"

void ioapic_set_entry(uint32_t gsi, uint8_t vector, uint16_t flags)
{
    uint32_t low = vector;

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

    // destination = local APIC ID 0 (BSP)
    uint32_t high = (0 << 24);

    ioapic_write(IOAPIC_REG_REDTBL + gsi * 2, low);
    ioapic_write(IOAPIC_REG_REDTBL + gsi * 2 + 1, high);
}

void ioapic_unmask(uint32_t irq)
{
    uint32_t low = ioapic_read(IOAPIC_REG_REDTBL + irq * 2);
    ioapic_write(IOAPIC_REG_REDTBL + irq * 2, low & ~(1 << 16));
}

void ioapic_mask(uint32_t irq)
{
    uint32_t low = ioapic_read(IOAPIC_REG_REDTBL + irq * 2);
    ioapic_write(IOAPIC_REG_REDTBL + irq * 2, low | (1 << 16));
}

uint32_t ioapic_is_mask(uint32_t irq)
{
    uint32_t low = ioapic_read(IOAPIC_REG_REDTBL + irq * 2);
    return (low >> 16) & 1;
}