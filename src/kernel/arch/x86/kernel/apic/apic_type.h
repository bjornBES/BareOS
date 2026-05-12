/*
 * File: apic_type.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "madt_type.h"
#include "kernel/smp/cpu_config.h"
#include "kernel.h"

typedef struct apic_priv
{
    uint64_t local_apic_base;
    uint64_t io_apic_base;
    iso_entry isos[16];
    int iso_count;

    // in apic_priv_t
    madt_cpu_t cpus[MAX_CPUS];
    uint8_t cpu_count;
    uint8_t bsp_apic_id; // bootstrap processor, skip during SIPI
} apic_priv_t;