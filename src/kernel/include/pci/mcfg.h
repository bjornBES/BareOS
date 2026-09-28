/*
 * File: mcfg.h
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "acpi/table.h"

#include <types.h>

typedef struct mcfg_entry
{
    uint64_t addr;
    uint16_t pci_segment_group;
    uint8_t start_pci_bus;
    uint8_t end_pci_bus;
    uint32_t reserved;
} PACKED mcfg_entry_t;

typedef struct
{
    sdt_header_t h;

    uint64_t reserved;

    mcfg_entry_t entries[];
} PACKED mcfg_t;

status_t mcfg_parse();
