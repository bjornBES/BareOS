/*
 * File: bp_acpi.h
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */



#pragma once

#include <stdint.h>
#include "defs.h"

typedef struct bp_acpi
{
    uint64_t rsdp_address;
} PACKED bp_acpi_t;
