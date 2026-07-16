/*
 * File: fadt.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fadt.h"
#include "acpi/table.h"
#include "acpi/tables/dsdt.h"
#include "kernel/acpi/fadt.h"
#include "kernel.h"
#include "debug/debug.h"
#include "mm/mmu/mmu.h"

#include "kernel/io.h"

#define MODULE "fadt"

#define FADT_FLAG_RESET_REG_SUP (1 << 10)

fadt_t *fadt = NULL;

void acpi_shutdown()
{
    if (!fadt)
    {
        return;
    }
    
    fadt_arch_shutdown(fadt);

    /*
    log_info(MODULE, "pm_1a=0x%x pm_1b=0x%x slp_typa=0x%x",
             fadt->pm_1a_control_block,
             fadt->pm_1b_control_block,
             slp_typa);

    uint16_t value = (slp_typa << 10) | (1 << 13);
    log_info(MODULE, "writing 0x%x to pm_1a", value);

    outw(fadt->pm_1a_control_block, value);

    if (fadt->pm_1b_control_block)
        outw(fadt->pm_1b_control_block, value);

    log_crit(MODULE, "shutdown failed"); */
}

void acpi_reboot()
{
    if (!fadt)
    {
        return;
    }

    fadt_arch_reboot(fadt);

    /*
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
        } */
}

void fadt_init()
{
    fadt = (fadt_t *)table_get_table(0x46414350);
    if (!fadt)
    {
        log_crit(MODULE, "no fadt_t found");
        return;
    }
    log_info(MODULE, "fadt_t found revision=%u", fadt->h.revision);

    table_verify_checksum(&fadt->h);

    fadt_arch_init(fadt);

    paddr_t dsdt_phys = fadt->x_dsdt ? fadt->x_dsdt : fadt->dsdt;
    dsdt_init(dsdt_phys);
}
