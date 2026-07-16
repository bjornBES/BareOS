/*
 * File: fadt.c
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/tables/fadt.h"
#include "acpi/aml/aml.h"
#include "kernel/irq.h"

uint32_t pm1a_cnt;
uint32_t pm1b_cnt;
generic_address_structure reset_register;

void acpi_sci_handler(intr_frame_t *frame, void *ctx)
{
}

void fadt_arch_init(fadt_t *fadt)
{
    if (fadt->smi_command_port && fadt->acpi_enable)
    {
        outb(fadt->smi_command_port, fadt->acpi_enable);
        // wait for ACPI mode
        while (!(inw(fadt->pm_1a_control_block) & 1));
    }

    // register SCI interrupt handler
    irq_arch_register(fadt->sci_interrupt, acpi_sci_handler, NULL);

    // save PM1 port addresses for power management
    pm1a_cnt = fadt->pm_1a_control_block;
    pm1b_cnt = fadt->pm_1b_control_block;
    reset_register = fadt->reset_reg;
}

void fadt_arch_shutdown(fadt_t *fadt)
{
    if (!fadt)
    {
        return;
    }
/*     outw(pm1a_cnt, slp_typa | SLP_EN);
    if (pm1b_cnt)
    {
        outw(pm1b_cnt, slp_typb | SLP_EN);
    } */
}

void fadt_arch_reboot(fadt_t *fadt)
{
    if (!fadt)
    {
        return;
    }
    if (reset_register.address_space == 0x01) // I/O port
    {
        outb((uint16_t)reset_register.address, fadt->reset_value);
    }
    else if (reset_register.address_space == 0x00) // MMIO
    {
        *(volatile uint8_t *)reset_register.address = fadt->reset_value;
    }
}
