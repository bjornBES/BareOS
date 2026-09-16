/*
 * File: lapic.c
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/acpi/apic/lapic.h"

vaddr_t local_apic_base;

void lapic_enable()
{
    // enable local APIC, set spurious vector to 0xFF
    lapic_write(LAPIC_REG_SPURIOUS, lapic_read(LAPIC_REG_SPURIOUS) | LAPIC_SPURIOUS_ENABLE | 0xFF);
}

void lapic_wait_idle()
{
    while (lapic_read(LAPIC_REG_ICR_LOW) & (1 << 12));
}


