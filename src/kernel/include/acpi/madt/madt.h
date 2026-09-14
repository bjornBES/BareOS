/*
 * File: madt.h
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "acpi/table.h"

#include <types.h>

typedef struct madt
{
    sdt_header_t header;
    uint32_t local_interrupt_address;
    uint32_t flags;
    uint8_t entries[];
} PACKED madt_t;

int madt_parse();
