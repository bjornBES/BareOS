/*
 * File: smp.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "cpu.h"
#include "kernel/asm/acpi/madt/madt_type.h"

typedef struct smp_cpu_entry
{
    uint8_t apic_id;
    uint8_t processor_id;
    bool enabled;
    bool is_bsp; // bootstrap processor
    cpu_t *cpu;  // filled later during bringup
} smp_cpu_entry_t;

// called by madt parser for each Local APIC entry
void smp_arch_register_cpu(madt_local_apic_t *entry);

// bringup — called by kernel smp init
void smp_arch_init(boot_params_t *bp);         // boot all registered APs
void smp_arch_boot_ap(smp_cpu_entry_t *entry); // boot one AP

// IPI
void smp_arch_send_ipi(uint8_t apic_id, uint8_t vector);
void smp_arch_send_ipi_all(uint8_t vector);    // broadcast
void smp_arch_send_ipi_others(uint8_t vector); // all except self

// identity
uint8_t smp_arch_current_apic_id();
uint32_t smp_arch_cpu_count();
