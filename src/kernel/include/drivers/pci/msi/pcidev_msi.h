/*
 * File: pcidev_msi.h
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/pci/pci.h"

#include <types.h>

status_t pci_msi_init(pci_device_t *pci_dev);
status_t pci_msix_init(pci_device_t *pci_dev);