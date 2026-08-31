/*
 * File: idt.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <defs.h>

#define IST_NONE 0 // normal RSP0
#define IST_NMI  1 // NMI — can interrupt anything including kernel
#define IST_DF   2 // double fault — stack might be corrupt
#define IST_MCE  3 // machine check — hardware error

typedef enum
{
    IDT_FLAG_GATE_TASK = 0x5,
    IDT_FLAG_GATE_16BIT_INT = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP = 0x7,
    IDT_FLAG_GATE_32BIT_INT = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP = 0xF,
    IDT_FLAG_GATE_64BIT_INT = 0xE,
    IDT_FLAG_GATE_64BIT_TRAP = 0xF,

    IDT_FLAG_RING0 = (0 << 5),
    IDT_FLAG_RING1 = (1 << 5),
    IDT_FLAG_RING2 = (2 << 5),
    IDT_FLAG_RING3 = (3 << 5),

    IDT_FLAG_PRESENT = 0x80,

} IDT_FLAGS;

#ifdef __x86_64__
typedef struct idt_entry
{
    uint16_t base_1;
    uint16_t segment_selector;
    uint8_t ist : 2;
    uint8_t reserved1 : 6;
    uint8_t flags;
    uint16_t base_2;
    uint32_t base_3;
    uint32_t reserved2;
} PACKED idt_entry_t;
#else
typedef struct idt_entry
{
    uint16_t base_1;
    uint16_t segment_selector;
    uint8_t reserved1;
    uint8_t flags;
    uint16_t base_2;
} PACKED idt_entry_t;
#endif

typedef struct
{
    uint16_t size;
    idt_entry_t *ptr;
} PACKED idtr_t;

void idt_load();

void idt_disable_gate(uint32_t interrupt);
void idt_enable_gate(uint32_t interrupt);

void idt_set_gate(uint32_t interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t ist);

void idt_initialize();

idt_entry_t *idt_get_entry(uint16_t sel);

void idt_dump_selector(uint16_t sel);
void idt_dump_selector_index(uint16_t index);

