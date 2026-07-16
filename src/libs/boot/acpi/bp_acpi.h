/*
 * File: bp_acpi.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "types.h"

typedef struct bp_acpi
{
    uint64_t rsdp_address;
} __attribute__((packed)) bp_acpi_t;
