/*
 * File: gdt.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include "kernel/memory.h"
#include "segments.h"
#include <defs.h>


typedef enum
{
    GDT_ACCESS_CODE_READABLE = 0x02,
    GDT_ACCESS_DATA_WRITEABLE = 0x02,

    GDT_ACCESS_CODE_CONFORMING = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN = 0x04,

    GDT_ACCESS_DATA_SEGMENT = 0x10,
    GDT_ACCESS_CODE_SEGMENT = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS = 0x00,

    GDT_ACCESS_RING0 = 0x00,
    GDT_ACCESS_RING1 = 0x20,
    GDT_ACCESS_RING2 = 0x40,
    GDT_ACCESS_RING3 = 0x60,

    GDT_ACCESS_PRESENT = 0x80,

} GDT_ACCESS;

typedef enum
{
    GDT_FLAG_64BIT = 0x20,
    GDT_FLAG_32BIT = 0x40,
    GDT_FLAG_16BIT = 0x00,

    GDT_FLAG_GRANULARITY_1B = 0x00,
    GDT_FLAG_GRANULARITY_4K = 0x80,
} GDT_FLAGS;

typedef struct
{
    uint16_t LimitLow;    // limit (bits 0-15)
    uint16_t BaseLow;     // base (bits 0-15)
    uint8_t BaseMiddle;   // base (bits 16-23)
    uint8_t Access;       // access
    uint8_t FlagsLimitHi; // limit (bits 16-19) | flags
    uint8_t BaseHigh;     // base (bits 24-31)
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
    uint16_t limit; // sizeof(gdt) - 1

    // @0xffffffff8002fb20
    gdt_entry_t *Ptr;  // address of GDT
} __attribute__((packed)) gdtr_t;
// at 0xffffffff8001b038
// result
// 0xf000ff53
// 0xf000e2c3

#define GDT_ENTRIES 12

extern gdt_entry_t gdt_table[GDT_ENTRIES];
extern gdtr_t gdt_descriptor;

#ifdef __i686__
void ASMCALL32 gdt_load_32(gdtr_t *descriptor);
#else
void gdt_load_64(gdtr_t *descriptor);
#endif

void x86_GDT_load(gdtr_t *gdt, gdt_entry_t* table);
void x86_GDT_set_entry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void x86_GDT_initialize();

// debug function
#ifdef DEBUG

void x86_GDT_dump_selector(uint16_t sel);

#endif
