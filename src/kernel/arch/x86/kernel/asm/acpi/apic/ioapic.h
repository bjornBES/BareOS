/*
 * File: ioapic.h
 * File Created: 19 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "apic.h"
#include "apic_type.h"
#include "kernel.h"

#define IOAPIC_REG_SELECT 0x00 // write register index here
#define IOAPIC_REG_WINDOW 0x10 // read/write data here

#define IOAPIC_REG_REDTBL 0x10 // redirection table starts at index 0x10 each entry is 2x 32-bit registers (low, high)


extern apic_priv_t *priv;

static inline void ioapic_write(uint32_t reg, uint32_t value)
{
    *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_SELECT) = reg;
    *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_WINDOW) = value;
}

static inline uint32_t ioapic_read(uint32_t reg)
{
    *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_SELECT) = reg;
    return *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_WINDOW);
}

void ioapic_unmask(uint32_t irq);
void ioapic_mask(uint32_t irq);
uint32_t ioapic_is_mask(uint32_t irq);
void ioapic_set_entry(uint32_t gsi, uint8_t vector, uint16_t flags);