/*
 * File: dsdt.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "acpi/table.h"

typedef struct dsdt
{
    sdt_header h;
    uint8_t *aml;
} dsdt_t;

void dsdt_init(paddr_t dsdt_phys);
