/*
 * File: madt_type.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "kernel/ACPI/ACPI.h"

typedef struct
{
    uint8_t source;
    uint32_t gsi;
    uint16_t flags;
} iso_entry;

typedef struct
{
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} madt_cpu_t;

// in apic.h or a new madt.h
typedef struct
{
    SDT_header header;
    uint32_t local_apic_address; // default local APIC MMIO base
    uint32_t flags;              // bit 0 = dual 8259 PICs present
    uint8_t entries[];           // variable length entries follow
} __attribute__((packed)) MADT;

// type 0 — processor local APIC
typedef struct
{
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags; // bit 0 = enabled
} __attribute__((packed)) MADT_local_apic;

// type 1 — I/O APIC
typedef struct
{
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address; // MMIO base
    uint32_t global_system_interrupt_base;
} __attribute__((packed)) MADT_io_apic;

// type 2 — interrupt source override (important!)
typedef struct
{
    uint8_t bus;
    uint8_t source;                   // ISA IRQ number
    uint32_t global_system_interrupt; // mapped GSI
    uint16_t flags;
} __attribute__((packed)) MADT_iso;

// entry header — every entry starts with these two bytes
typedef struct
{
    uint8_t type;
    uint8_t length;
    union
    {
        MADT_local_apic local;
        MADT_io_apic io;
        MADT_iso iso;
    } __attribute__((packed));
} __attribute__((packed)) MADT_entry;