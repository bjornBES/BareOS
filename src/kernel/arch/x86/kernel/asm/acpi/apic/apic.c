/*
 * File: apic.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "apic.h"

#include "kernel.h"
#include "kernel/irq.h"
#include "kernel/asm/irq/irq.h"
#include "kernel/mmu.h"
#include "kernel/memory.h"

#include "debug/debug.h"

#include "mm/ioremap/ioremap.h"
#include "mm/memdefs.h"

#include "lapic.h"
#include "ioapic.h"

#define MODULE "APIC"

#define IOAPIC_REG_SELECT 0x00 // write register index here
#define IOAPIC_REG_WINDOW 0x10 // read/write data here

#define IOAPIC_REG_REDTBL 0x10 // redirection table starts at index 0x10
                               // each entry is 2x 32-bit registers (low, high)

apic_priv_t *priv;

void apic_check_priv()
{
    if (priv == NULL)
    {
        priv = malloc(sizeof(apic_priv_t));
    }
}

void apic_set_local_base(paddr_t lapic_phys_base)
{
    apic_check_priv();
    priv->local_apic_base = ioremap(lapic_phys_base, PAGE_SIZE);
}

void apic_set_io_base(paddr_t ioapic_phys_base)
{
    apic_check_priv();
    priv->io_apic_base = ioremap(ioapic_phys_base, PAGE_SIZE);
}

uint32_t get_gsi(uint32_t irq)
{
    for (int i = 0; i < priv->iso_count; i++)
    {
        if (priv->isos[i].source == irq)
        {
            return priv->isos[i].gsi;
        }
    }
    return irq; // no override, GSI == IRQ
}

static uint16_t get_iso_flags(uint32_t irq)
{
    for (int i = 0; i < priv->iso_count; i++)
    {
        if (priv->isos[i].source == irq)
        {
            return priv->isos[i].flags;
        }
    }
    return 0;
}

void irq_arch_eoi(uint32_t _1)
{
    // log_debug(MODULE, "got eoi from %u", _1);
    lapic_write(LAPIC_REG_EOI, 0);
}

void apic_unmask(uint32_t irq)
{
    ioapic_unmask(irq);
}

void apic_mask(uint32_t irq)
{
    ioapic_mask(irq);
}

uint32_t apic_is_masked(uint32_t irq)
{
    return ioapic_is_mask(irq);
}

uint32_t apic_get_gsi(uint32_t irq)
{
    return get_gsi(irq);
}

void apic_Initialize(uint8_t offset_pic_1, uint8_t _2, bool _3)
{
    // enable local APIC, set spurious vector to 0xFF
    lapic_enable();
    log_info(MODULE, "local APIC enabled");

    for (uint8_t irq = 0; irq < 24; irq++)
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
    if (priv == NULL)
    {
        priv = malloc(sizeof(apic_priv_t));
    }
    if (!priv)
    {
        log_crit(MODULE, "malloc failed");
        return false;
    }
    madt_parse();
    return priv->local_apic_base != 0 && priv->io_apic_base != 0;
}

void irq_arch_register_override(uint32_t gsi, uint8_t source_irq, uint16_t flags)
{
    if (priv->iso_count < 24)
    {
        priv->isos[priv->iso_count].source = source_irq;
        priv->isos[priv->iso_count].gsi = gsi;
        priv->isos[priv->iso_count].flags = flags;
        priv->iso_count++;
    }
}

static const pic_driver apic_driver = {
    .name = "APIC",
    .probe = apic_probe,
    .initialize = apic_Initialize,
    .disable = NULL,
    .send_eoi = irq_arch_eoi,
    .mask = apic_mask,
    .unmask = apic_unmask,
    .is_masked = apic_is_masked,
    .get_real_irq = apic_get_gsi,
};

const pic_driver *apic_get_driver()
{
    return &apic_driver;
}

