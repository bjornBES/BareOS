/*
 * File: apic.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "apic_type.h"
#include "kernel/asm/irq/pic.h"
#include "kernel/madt.h"

typedef uint16_t lapic_id;

extern apic_priv_t *priv;

void apic_set_io_base(paddr_t ioapic_phys_base);
void apic_set_local_base(paddr_t lapic_phys_base);
const pic_driver *apic_get_driver();
