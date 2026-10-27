/*
 * File: msi.h
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/pci/pci.h"

status_t msi_pci_read8(pci_device_t *dev, uint16_t offset, uint8_t *out);
status_t msi_pci_read16(pci_device_t *dev, uint16_t offset, uint16_t *out);
status_t msi_pci_read32(pci_device_t *dev, uint16_t offset, uint32_t *out);

status_t msi_pci_write8(pci_device_t *dev, uint16_t offset, uint8_t val);
status_t msi_pci_write16(pci_device_t *dev, uint16_t offset, uint16_t val);
status_t msi_pci_write32(pci_device_t *dev, uint16_t offset, uint32_t val);

status_t pci_enable_msi_range(pci_device_t *dev, size_t min_vecs, size_t max_vecs);

status_t msi_enable(pci_device_t *dev);
status_t msi_disable(pci_device_t *dev);