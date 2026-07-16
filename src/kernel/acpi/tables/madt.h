/*
 * File: madt.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "acpi/table.h"

typedef struct
{
    sdt_header header;
    uint32_t local_apic_address;
    uint32_t flags;
    uint8_t entries[];
} __attribute__((packed)) madt;

void madt_parse();