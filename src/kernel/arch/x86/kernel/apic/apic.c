/*
 * File: apic.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "apic.h"
#include "kernel/irq.h"
#include "kernel.h"
#include "kernel/ACPI/ACPI.h"
#include "debug/debug.h"

#include "memory/ioremap/ioremap.h"
#include "memory/paging/paging.h"
#include "memory/memdefs.h"
#include "libs/malloc.h"

#include "lapic.h"

#define MODULE "APIC"

#define IOAPIC_REG_SELECT 0x00 // write register index here
#define IOAPIC_REG_WINDOW 0x10 // read/write data here

#define IOAPIC_REG_REDTBL 0x10 // redirection table starts at index 0x10
                               // each entry is 2x 32-bit registers (low, high)

apic_priv_t *priv;

static void ioapic_write(uint32_t reg, uint32_t value)
{
    *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_SELECT) = reg;
    *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_WINDOW) = value;
}

static uint32_t ioapic_read(uint32_t reg)
{
    *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_SELECT) = reg;
    return *(volatile uint32_t *)(priv->io_apic_base + IOAPIC_REG_WINDOW);
}

static uint32_t get_gsi(uint8_t irq)
{
    for (int i = 0; i < priv->iso_count; i++)
        if (priv->isos[i].source == irq)
            return priv->isos[i].gsi;
    return irq; // no override, GSI == IRQ
}

static uint16_t get_iso_flags(uint8_t irq)
{
    for (int i = 0; i < priv->iso_count; i++)
        if (priv->isos[i].source == irq)
            return priv->isos[i].flags;
    return 0;
}

static void ioapic_set_entry(uint32_t gsi, uint8_t vector, uint16_t flags)
{
    uint32_t low = vector;

    // polarity — bit 1 of flags, 1 = active low
    if (flags & 0x2)
        low |= (1 << 13);

    // trigger mode — bit 3 of flags, 1 = level triggered
    if (flags & 0x8)
        low |= (1 << 15);

    // start masked
    low |= (1 << 16);

    // destination = local APIC ID 0 (BSP)
    uint32_t high = (0 << 24);

    ioapic_write(IOAPIC_REG_REDTBL + gsi * 2, low);
    ioapic_write(IOAPIC_REG_REDTBL + gsi * 2 + 1, high);
}

void apic_send_eoi(int _1)
{
    // log_debug(MODULE, "got eoi from %u", _1);
    lapic_write(LAPIC_REG_EOI, 0);
}

void apic_unmask(int irq)
{
    uint32_t gsi = get_gsi(irq);
    uint32_t low = ioapic_read(IOAPIC_REG_REDTBL + gsi * 2);
    ioapic_write(IOAPIC_REG_REDTBL + gsi * 2, low & ~(1 << 16));
}

void apic_mask(int irq)
{
    uint32_t gsi = get_gsi(irq);
    uint32_t low = ioapic_read(IOAPIC_REG_REDTBL + gsi * 2);
    ioapic_write(IOAPIC_REG_REDTBL + gsi * 2, low | (1 << 16));
}

uint32_t apic_get_gsi(uint8_t irq)
{
    return get_gsi(irq);
}

void apic_Initialize(uint8_t _1, uint8_t _2, bool _3)
{
    priv->local_apic_base = (uint64_t)ioremap((phys_addr)priv->local_apic_base, PAGE_SIZE);

    priv->io_apic_base = (uint64_t)ioremap((phys_addr)priv->io_apic_base, PAGE_SIZE);

    // enable local APIC, set spurious vector to 0xFF
    lapic_enable();
    log_info(MODULE, "local APIC enabled");

    for (uint8_t irq = 0; irq < 16; irq++)
    {
        uint32_t gsi = get_gsi(irq);
        uint16_t flags = get_iso_flags(irq);
        uint8_t vector = PIC_REMAP_OFFSET + irq;
        ioapic_set_entry(gsi, vector, flags);
        log_info(MODULE, "IRQ %u -> GSI %u vector 0x%x", irq, gsi, vector);
    }
}

bool apic_probe()
{
    priv = malloc(sizeof(apic_priv_t));
    if (!priv)
    {
        log_crit(MODULE, "malloc failed");
        return false;
    }
    MADT_parse_madt(priv);
    return priv->local_apic_base != 0 && priv->io_apic_base != 0;
}

static const pic_driver apic_driver = {
    .name = "APIC",
    .probe = apic_probe,
    .initialize = apic_Initialize,
    .disable = NULL,
    .send_eoi = apic_send_eoi,
    .mask = apic_mask,
    .unmask = apic_unmask,
};

const pic_driver *apic_get_driver()
{
    return &apic_driver;
}

