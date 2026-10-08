/*
 * File: ioapic.h
 * File Created: 19 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "irq/irq_controller.h"

#include <types.h>

#define IOAPIC_REG_SELECT 0x00 // write register index here
#define IOAPIC_REG_WINDOW 0x10 // read/write data here

#define IOAPIC_REG_VERSION 0x01
#define IOAPIC_REG_REDTBL 0x10 // redirection table starts at index 0x10 each entry is 2x 32-bit registers (low, high)

typedef struct iso_entry
{
    interrupt_vector_t vector;
    gsi_t gsi;
    kernel_irq_t irq;
    uint8_t pin;
    uint16_t flags;
    uint32_t lapic_target;
    bool locked;
    bool dont_use;
    irq_trigger_t trigger;
    irq_polarity_t polarity;
} iso_entry_t;

typedef struct ioapic_entry
{
    uint8_t ioapic_id;
    vaddr_t io_apic_base;
    gsi_t gsi_base;
    gsi_t gsi_end;
    uint16_t ioapic_max_redir;
    iso_entry_t redir_entries[24];
} ioapic_entry_t;

extern uint8_t ioapic_count;

status_t ioapic_print_entry(uint8_t ioapic_id, gsi_t gsi);

/// @brief Will register a new io apic entry
/// @param ioapic_id The ID of the given io apic, given from madt entry type 1
/// @param ioapic_address The address of the io acpi, given from madt
/// @param gsi_base The start gsi index of the give io acpi, given from madt
/// @return KERRNO_SUCCESSES on successes or an kerrno number
status_t ioapic_register(uint8_t ioapic_id, paddr_t ioapic_address, uint32_t gsi_base);

void ioapic_write(uint8_t ioapic_id, uint32_t reg, uint32_t value);
uint32_t ioapic_read(uint8_t ioapic_id, uint32_t reg);

ioapic_entry_t *ioapic_get_entry(uint8_t ioapic_id);

uint8_t ioapic_get_id(gsi_t gsi);

void ioapic_unmask(uint8_t ioapic_id, uint32_t irq);
void ioapic_mask(uint8_t ioapic_id, uint32_t irq);
uint32_t ioapic_is_mask(uint8_t ioapic_id, uint32_t irq);
void ioapic_set_overwrite(uint8_t ioapic_id, gsi_t gsi, uint8_t vector, uint16_t flags, uint32_t dest_apic_id);
void ioapic_set_entry(uint8_t ioapic_id, gsi_t gsi, uint8_t irq, irq_trigger_t trigger, irq_polarity_t polarity, uint32_t dest_apic_id);
