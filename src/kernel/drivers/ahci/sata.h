/*
 * File: sata.h
 * File Created: 08 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "ahci.h"

#include "dev/devicet.h"

#include "drivers/pci/pci_dev.h"

#include <types.h>

uint32_t sata_find_cmdslot(ahci_port_priv_t *aport);
status_t sata_init_port(pci_device_t *dev, ahci_host_priv_t *host, ahci_port_priv_t *aport, device_t **out_dev);