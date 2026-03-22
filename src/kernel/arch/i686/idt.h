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
#include <stdio.h>

typedef enum
{
    IDT_FLAG_GATE_TASK              = 0x5,
    IDT_FLAG_GATE_16BIT_INT         = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP        = 0x7,
    IDT_FLAG_GATE_32BIT_INT         = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP        = 0xF,

    IDT_FLAG_RING0                  = (0 << 5),
    IDT_FLAG_RING1                  = (1 << 5),
    IDT_FLAG_RING2                  = (2 << 5),
    IDT_FLAG_RING3                  = (3 << 5),

    IDT_FLAG_PRESENT                = 0x80,

} IDT_FLAGS;

typedef struct
{
    uint16_t BaseLow;
    uint16_t SegmentSelector;
    uint8_t Reserved;
    uint8_t Flags;
    uint16_t BaseHigh;
} __attribute__((packed)) IDTEntry;

typedef struct
{
    uint16_t Limit;
    IDTEntry* Ptr;
} __attribute__((packed)) IDTDescriptor;

void i686_IDTLoad();

void i686_IDTDisableGate(int interrupt);
void i686_IDTEnableGate(int interrupt);
void i686_IDTSetGate(int interrupt, void* base, uint32_t segmentDescriptor, uint8_t flags);

void i686_IDTInitialize();

// debug function
#ifdef DEBUG

void i686_IDT_DumpSelector(uint16_t sel);

#endif
