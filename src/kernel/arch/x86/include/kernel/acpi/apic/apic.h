/*
 * File: apic.h
 * File Created: 15 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "irq/irq.h"

#include <types.h>

extern bool enabled_x2apic;

irq_controller_t *apic_get_driver();
