/*
 * File: FADT.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "FADT.h"
#include "kernel.h"
#include "debug/debug.h"
#include "memory/paging/paging.h"

#include "libs/IO.h"

#define MODULE "FADT"

#define FADT_FLAG_RESET_REG_SUP (1 << 10)

static FADT *fadt = NULL;
static uint16_t slp_typa;

static uint16_t FADT_get_slp_typa()
{
    uint64_t dsdt_phys = (fadt->h.revision >= 2 && fadt->x_dsdt)
                             ? fadt->x_dsdt
                             : (uint64_t)fadt->dsdt;

    uint64_t addr = ALIGN_2_DOWN((uint32_64)dsdt_phys, PAGE_SIZE);
    paging_map_region(kernel_page, (virt_addr)addr, (phys_addr)addr, PAGE_SIZE, mmio_flags);

    SDT_header *dsdt = (SDT_header *)dsdt_phys;
    uint8_t *aml = (uint8_t *)dsdt + sizeof(SDT_header);
    uint32_t aml_len = dsdt->length - sizeof(SDT_header);
    paging_map_region(kernel_page, (virt_addr)addr, (phys_addr)addr, dsdt->length, mmio_flags);

    // scan for "_S5_" signature
    for (uint32_t i = 0; i < aml_len - 8; i++)
    {
        if (aml[i] == '_' && aml[i + 1] == 'S' &&
            aml[i + 2] == '5' && aml[i + 3] == '_')
        {
            return (uint16_t)aml[i + 7];
        }
    }

    log_warn(MODULE, "_S5 not found in DSDT, defaulting to 0");
    return 0;
}

void FADT_shutdown(void)
{
    if (!fadt)
        return;

    log_info(MODULE, "pm_1a=0x%x pm_1b=0x%x slp_typa=0x%x",
             fadt->pm_1a_control_block,
             fadt->pm_1b_control_block,
             slp_typa);

    uint16_t value = (slp_typa << 10) | (1 << 13);
    log_info(MODULE, "writing 0x%x to pm_1a", value);

    outw(fadt->pm_1a_control_block, value);

    if (fadt->pm_1b_control_block)
        outw(fadt->pm_1b_control_block, value);

    log_crit(MODULE, "shutdown failed");
}

void FADT_reset(void)
{
    if (!fadt)
        return;

    if (fadt->h.revision < 2)
    {
        log_warn(MODULE, "ACPI reset not supported on revision 1, falling back");
        // fallback: keyboard controller reset
        outb(0x64, 0xFE);
        return;
    }

    if (!(fadt->flags & FADT_FLAG_RESET_REG_SUP))
    {
        log_warn(MODULE, "reset_reg not supported by firmware");
        outb(0x64, 0xFE);
        return;
    }

    // now safe to use reset_reg
    if (fadt->reset_reg.address_space == 0x01) // I/O port
    {
        outb((uint16_t)fadt->reset_reg.address, fadt->reset_value);
    }
    else if (fadt->reset_reg.address_space == 0x00) // MMIO
    {
        *(volatile uint8_t *)fadt->reset_reg.address = fadt->reset_value;
    }
}

void FADT_init()
{
    fadt = (FADT *)ACPI_get_table("FACP");
    if (!fadt)
    {
        log_crit(MODULE, "no FADT found");
        return;
    }
    log_info(MODULE, "FADT found revision=%u", fadt->h.revision);
    slp_typa = FADT_get_slp_typa();
}
