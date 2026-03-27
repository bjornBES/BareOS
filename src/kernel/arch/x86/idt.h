/*
 * File: idt.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "VFS/vfs.h"
#include "libs/stdio.h"

typedef enum
{
    IDT_FLAG_GATE_TASK              = 0x5,
    IDT_FLAG_GATE_16BIT_INT         = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP        = 0x7,
    IDT_FLAG_GATE_32BIT_INT         = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP        = 0xF,
    IDT_FLAG_GATE_64BIT_INT         = 0xE,
    IDT_FLAG_GATE_64BIT_TRAP        = 0xF,

    IDT_FLAG_RING0                  = (0 << 5),
    IDT_FLAG_RING1                  = (1 << 5),
    IDT_FLAG_RING2                  = (2 << 5),
    IDT_FLAG_RING3                  = (3 << 5),

    IDT_FLAG_PRESENT                = 0x80,

} IDT_FLAGS;

typedef struct
{
    uint16_t base_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) IDT_entry32;

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
} __attribute__((packed)) IDT_entry64;

#ifdef __i686__
typedef IDT_entry32 IDT_entry;
#else
typedef IDT_entry64 IDT_entry;
#endif

typedef struct
{
    uint16_t size;
    IDT_entry* ptr;
} __attribute__((packed)) IDT_descriptor;

void x86_IDT_load();

void x86_IDT_disable_gate(int interrupt);
void x86_IDT_enable_gate(int interrupt);
#ifdef __i686__
void x86_IDT_set_gate(int interrupt, void* base, uint32_t segment_descriptor, uint8_t flags);
#else
void x86_IDT_set_gate(int interrupt, void* base, uint32_t segment_descriptor, uint8_t flags, uint8_t ist);
#endif

void x86_IDT_initialize();

// debug function
#ifdef DEBUG

void x86_IDT_dump_selector(uint16_t sel);

#endif
