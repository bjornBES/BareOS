/*
 * File: idt.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "VFS/vfs.h"
#include "stdio.h"

#define IST_NONE 0 // normal RSP0
#define IST_NMI 1  // NMI — can interrupt anything including kernel
#define IST_DF 2   // double fault — stack might be corrupt
#define IST_MCE 3  // machine check — hardware error

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

typedef struct
{
    uint16_t base_1;
    uint16_t segment_selector;
    uint8_t reserved1;
    uint8_t flags;
    uint16_t base_2;
} __attribute__((packed)) idt_entry32;

typedef struct
{
    uint16_t base_1;
    uint16_t segment_selector;
    uint8_t ist : 2;
    uint8_t reserved1 : 6;
    uint8_t flags;
    uint16_t base_2;
    uint32_t base_3;
    uint32_t reserved2;
} __attribute__((packed)) idt_entry64;

#ifdef __i686__
typedef idt_entry32 idt_entry;
#else
typedef idt_entry64 idt_entry;
#endif

typedef struct
{
    uint16_t size;
    idt_entry *ptr;
} __attribute__((packed)) idtr_t;

extern idt_entry kernel_idt_table[256];
extern idtr_t kernel_idtr;

void x86_idt_load();

void x86_idt_disable_gate(idt_entry table[256], uint32_t interrupt);
void x86_idt_enable_gate(idt_entry table[256], uint32_t interrupt);
#ifdef __i686__
void x86_idt_set_gate(uint32_t interrupt, void *base, uint32_t segment_descriptor, uint8_t flags, uint8_t res);
void idt_set_gate(idt_entry table[256], uint32_t interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t res);
#else
void x86_idt_set_gate(uint32_t interrupt, void *base, uint32_t segment_descriptor, uint8_t flags, uint8_t ist);
void idt_set_gate(idt_entry table[256], uint32_t interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t ist);
#endif

void x86_idt_initialize();

// debug function
#ifdef DEBUG

void x86_idt_dump_selector(uint16_t sel);

#endif
