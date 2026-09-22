/*
 * File: madt.c
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/madt_arch.h"
#include "kernel/acpi/apic/apic.h"
#include "kernel/acpi/apic/lapic.h"
#include "kernel/acpi/apic/ioapic.h"

#include "kernel/cpuid/cpuid.h"
#include "kernel/msr/msr.h"
#include "x86_arch_data.h"

#include "debug/debug.h"

#include "mm/ioremap.h"

#include "cpu/cpu.h"

#include <types.h>
#include <binary.h>

#define MODULE "x86-madt"

// type 0 — processor local APIC
typedef struct madt_local_apic
{
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags; // bit 0 = enabled
} PACKED madt_local_apic_t;

// type 1 — I/O APIC
typedef struct
{
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address; // MMIO base
    uint32_t global_system_interrupt_base;
} PACKED madt_io_apic;

// type 2 — interrupt source override
typedef struct
{
    uint8_t bus;
    uint8_t source;                   // ISA IRQ number
    uint32_t global_system_interrupt; // mapped GSI
    uint16_t flags;
} PACKED madt_iso;

// type 4 — non-maskable interrupts
typedef struct
{
    uint8_t apic_id;
    uint16_t flags;
    uint8_t lint;
} PACKED madt_nmi;

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
        madt_nmi nmi;
    } PACKED;
} PACKED madt_entry;

extern x86_arch_data_t arch_runtime_data;

status_t madt_arch_parse(madt_t *madt)
{
    if (arch_runtime_data.cpuid.leaf_0x1_0->apic)
    {
        trace_info(MODULE, "enabled xapic");
        uint64_t apic_base = rdmsr(MSR_IA32_APIC_BASE);
        apic_base |= BIT(MSR_IA32_APIC_GLOBAL_ENABLE_BIT); // EN
        wrmsr(MSR_IA32_APIC_BASE, apic_base);
    }
    if (arch_runtime_data.cpuid.leaf_0x1_0->x2apic)
    {
        trace_info(MODULE, "enabled x2apic");
        uint64_t apic_base = rdmsr(MSR_IA32_APIC_BASE);
        apic_base |= BIT(MSR_IA32_X2APIC_ENABLE_BIT); // EXTD
        wrmsr(MSR_IA32_APIC_BASE, apic_base);
    }
    local_apic_base = ioremap(madt->local_interrupt_address, 1024);
    uint8_t *entry = madt->entries;
    uint8_t *end = (uint8_t *)madt + madt->header.length;

    uint32_t bsp_id = lapic_get_id();

    while (entry < end)
    {
        madt_entry *en = (madt_entry *)entry;
        switch (en->type)
        {
            case 0 : // local APIC
                {
                    madt_local_apic_t *la = (madt_local_apic_t *)&en->local;
                    trace_info(MODULE, "CPU %u APIC ID %u flags=%u", la->processor_id, la->apic_id, la->flags);

                    if (FLAG_IS_SET(la->flags, 1) == false)
                    {
                        log_err(MODULE, "CPU flags=0x2 not good");
                        for (;;)
                        {
                            ;
                        }
                    }

                    cpu_register(la->processor_id, la->apic_id, bsp_id == la->apic_id);
                    // smp_arch_register_cpu(la);
                    break;
                }
            case 1 : // I/O APIC
                {
                    madt_io_apic *ia = (madt_io_apic *)&en->io;
                    // apic_set_io_base((paddr_t)ia->io_apic_address);
                    trace_info(MODULE, "IOAPIC ID %u base=0x%x GSI base=%u", ia->io_apic_id, ia->io_apic_address, ia->global_system_interrupt_base);
                    ioapic_register(ia->io_apic_id, ia->io_apic_address, ia->global_system_interrupt_base);
                    break;
                }
            case 2 : // interrupt source override
                {
                    madt_iso *iso = (madt_iso *)&en->iso;
                    trace_info(MODULE, "IOAPIC ID %u ISO IRQ %u -> GSI %u flags=0x%x", iso->bus, iso->source, iso->global_system_interrupt, iso->flags);

                    // irq_arch_register_override(iso->global_system_interrupt, iso->source, iso->flags);
                    ioapic_set_entry(iso->bus, iso->global_system_interrupt, iso->source, iso->flags, 0);
                    break;
                }
            case 4 : // interrupt source override
                {
                    madt_nmi *nmi = (madt_nmi *)&en->nmi;
                    trace_info(MODULE, "NMI APIC ID %u, flags=0x%x, lint=%u", nmi->apic_id, nmi->flags, nmi->lint);

                    // irq_arch_register_override(iso->global_system_interrupt, iso->source, iso->flags);
                    break;
                }
            default :
                {
                    trace_info(MODULE, "Entry type = 0x%x, length = %u", en->type, en->length);
                }
                break;
        }
        entry += en->length; // advance by entry length, not sizeof
    }
    return KERRNO_SUCCESSES;
}
