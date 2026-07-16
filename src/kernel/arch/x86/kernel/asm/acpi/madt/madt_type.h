/*
 * File: madt_type.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

typedef struct
{
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} madt_cpu_t;

// type 0 — processor local APIC
typedef struct madt_local_apic
{
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags; // bit 0 = enabled
} __attribute__((packed)) madt_local_apic_t;

// type 1 — I/O APIC
typedef struct
{
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address; // MMIO base
    uint32_t global_system_interrupt_base;
} __attribute__((packed)) madt_io_apic;

// type 2 — interrupt source override (important!)
typedef struct
{
    uint8_t bus;
    uint8_t source;                   // ISA IRQ number
    uint32_t global_system_interrupt; // mapped GSI
    uint16_t flags;
} __attribute__((packed)) madt_iso;

// entry header — every entry starts with these two bytes
typedef struct
{
    uint8_t type;
    uint8_t length;
    union
    {
        madt_local_apic_t local;
        madt_io_apic io;
        madt_iso iso;
    } __attribute__((packed));
} __attribute__((packed)) madt_entry;