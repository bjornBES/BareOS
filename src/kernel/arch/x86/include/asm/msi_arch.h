/*
 * File: msi_arch.h
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "irq/irqt.h"

#include "drivers/pci/pci_dev.h"

#include <types.h>

status_t msi_arch_set_vector(pci_device_t *dev, interrupt_vector_t vector, cpu_logical_id_t target, irq_trigger_t trigger, irq_polarity_t polarity);
