/*
 * File: gdt.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <defs.h>

#include "segments.h"

typedef enum
{
    GDT_ACCESS_CODE_READABLE = 0x02,
    GDT_ACCESS_DATA_WRITEABLE = 0x02,

    GDT_ACCESS_CODE_EXE_IN_RING = 0x00,
    GDT_ACCESS_CODE_CONFORMING = 0x04,
    GDT_ACCESS_DATA_DIRECTION_UP = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN = 0x04,

    GDT_ACCESS_DATA_SEGMENT = 0x10,
    GDT_ACCESS_CODE_SEGMENT = 0x18,

    GDT_ACCESS_DESCRIPTOR_TYPE_TSS = 0x00,

    GDT_ACCESS_RING0 = 0x00, // kernel
    GDT_ACCESS_RING1 = 0x20,
    GDT_ACCESS_RING2 = 0x40,
    GDT_ACCESS_RING3 = 0x60, // user

    GDT_ACCESS_PRESENT = 0x80,

} GDT_ACCESS;

typedef enum
{
    GDT_FLAG_64BIT = 0x2,
    GDT_FLAG_32BIT = 0x4,
    GDT_FLAG_16BIT = 0x0,

    GDT_FLAG_GRANULARITY_1B = 0x0,
    GDT_FLAG_GRANULARITY_4K = 0x8,
} GDT_FLAGS;

typedef struct
{
    uint16_t limit_low;     // limit (bits 0-15)
    uint16_t base_low;      // base (bits 0-15)
    uint8_t base_middle;    // base (bits 16-23)
    uint8_t access;         // access
    uint8_t limit_high : 4; // limit (bits 16-19)
    uint8_t flags : 4;      // flags
    uint8_t base_high;      // base (bits 24-31)
} PACKED gdt_entry_t;

#ifdef __x86_64__
typedef struct
{
    uint16_t limit_low;     // limit (bits 0-15)
    uint16_t base_low;      // base (bits 0-15)
    uint8_t base_middle;    // base (bits 16-23)
    uint8_t access;         // 0x89 = present, ring0, TSS available
    uint8_t limit_high : 4; // limit (bits 16-19)
    uint8_t flags : 4;      // flags
    uint8_t base_high;      // base (bits 24-31)
    uint32_t base_upper;    // bits 63:32
    uint32_t reserved;
} PACKED system_segment_descriptor_t;
#else
typedef gdt_entry_t system_segment_descriptor_t;
#endif

typedef struct
{
    uint16_t limit;
    gdt_entry_t *ptr;
} PACKED gdtr_t;

#define GDT_ENTRIES 12

void gdt_set_entry(gdt_entry_t *table, uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_set_entry_system_segment(gdt_entry_t *table, uint16_t index, uint32_64 base, uint32_t limit, uint8_t access, uint8_t flags);

void gdt_load(gdtr_t *gdt, gdt_entry_t *table);
void gdt_initialize(gdtr_t *gdt, gdt_entry_t *table);

gdt_entry_t *gdt_get_entry(uint16_t sel);

void gdt_dump_selector(uint16_t sel);
void x86_gdt_dump_selector_index(uint16_t index);

