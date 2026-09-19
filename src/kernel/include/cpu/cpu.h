/*
 * File: cpu.h
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/cput_arch.h"

#include <types.h>

typedef enum cpu_state
{
    CPU_STATE_ABSENT = 0,
    CPU_STATE_OFFLINE,
    CPU_STATE_BOOTING,
    CPU_STATE_ONLINE,
} cpu_state_t;

typedef struct cpu_entry
{
    cpu_logical_id_t logical_id; // kernel-internal index, 0..N-1, what scheduler/percpu arrays index by
    uint32_t processor_uid;      // ACPI Processor UID, cross-ref DSDT _UID
    uint32_t arch_id;            // opaque addressing handle for arch layer (APIC ID on x86), never interpreted generically
    bool is_bsp;
    cpu_state_t state;           // enumerated but not yet started until AP bring-up phase
    cpu_t *cpu;
} cpu_entry_t;

int cpu_register(uint32_t processor_uid, uint32_t arch_id, bool is_bsp);
cpu_entry_t *cpu_get_entry(cpu_logical_id_t id);
cpu_entry_t *cpu_get_bsp();

