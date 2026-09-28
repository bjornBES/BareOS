/*
 * File: arch_pci.h
 * File Created: 23 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "pci/pci.h"

#include <types.h>

uint32_t pci_arch_read_config32(pci_device_t *dev, uint16_t offset);
int pci_arch_write_config32(pci_device_t *dev, uint16_t offset, uint32_t val);