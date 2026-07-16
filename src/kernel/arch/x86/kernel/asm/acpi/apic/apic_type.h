/*
 * File: apic_type.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "kernel/asm/acpi/madt/madt_type.h"
#include "kernel.h"

typedef struct
{
    uint8_t source;
    uint32_t gsi;
    uint16_t flags;
} iso_entry_t;

typedef struct apic_priv
{
    vaddr_t local_apic_base;
    vaddr_t io_apic_base;
    iso_entry_t isos[24];
    int iso_count;
} apic_priv_t;