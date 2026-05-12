/*
 * File: apic.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "apic_type.h"
#include "kernel/pic.h"
#include "MADT.h"

typedef uint16_t lapic_id;

extern apic_priv_t *priv;

uint32_t apic_get_gsi(uint8_t irq);
const pic_driver *apic_get_driver();