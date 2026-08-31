/*
 * File: gdt.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "gdt.h"

#include <defs.h>

#include "debug/debug.h"

// helper functions
#define GDT_LIMIT_LOW(limit)                  (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                    (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)                 ((base >> 16) & 0xFF)
#define GDT_LIMIT_HIGH(limit)                 ((limit >> 16) & 0xF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)      (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                   ((base >> 24) & 0xFF)
#define GDT_BASE_UPPER(base)                  ((base >> 32) & 0xFFFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    access,                                     \
    ((limit >> 16) & 0xF),                      \
    flags,                                      \
    GDT_BASE_HIGH(base)}

#ifdef __i686__
ASMCALL void gdt_load_32(gdtr_t *descriptor);
#else
ASMCALL void gdt_load_64(gdtr_t *descriptor);
#endif

void gdt_set_entry(gdt_entry_t *table, uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    gdt_entry_t *entry = &table[index];
    entry->access = access;
    entry->base_high = GDT_BASE_HIGH(base);
    entry->base_middle = GDT_BASE_MIDDLE(base);
    entry->base_low = GDT_BASE_LOW(base);
    entry->flags = flags;
    entry->limit_high = GDT_LIMIT_HIGH(limit);
    entry->limit_low = GDT_LIMIT_LOW(limit);
}

void gdt_set_entry_system_segment(gdt_entry_t *table, uint16_t index, uint32_64 base, uint32_t limit, uint8_t access, uint8_t flags)
{
    system_segment_descriptor_t *entry = (system_segment_descriptor_t *)&table[index];
    entry->access = access;
    entry->base_upper = GDT_BASE_UPPER(base);
    entry->base_high = GDT_BASE_HIGH(base);
    entry->base_middle = GDT_BASE_MIDDLE(base);
    entry->base_low = GDT_BASE_LOW(base);
    entry->flags = flags;
    entry->limit_high = GDT_LIMIT_HIGH(limit);
    entry->limit_low = GDT_LIMIT_LOW(limit);
    entry->reserved = 0;
}

void gdt_load(gdtr_t *gdt, gdt_entry_t *table)
{
    gdt->limit = sizeof(gdt_entry_t[GDT_ENTRIES]) - 1;
    gdt->ptr = table;
#ifdef __x86_64__
    gdt_load_64(gdt);
#else
    gdt_load_32(gdt);
#endif
}

void gdt_initialize(gdtr_t *gdt, gdt_entry_t *table)
{
    gdt_set_entry(table, 0, 0, 0, 0, 0);
#ifdef __x86_64__
    gdt_set_entry(table, KERNEL_CODE_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_CODE_READABLE | GDT_ACCESS_CODE_EXE_IN_RING | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_RING0 | GDT_ACCESS_PRESENT, GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, KERNEL_DATA_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_DATA_WRITEABLE | GDT_ACCESS_DATA_DIRECTION_UP | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_RING0 | GDT_ACCESS_PRESENT, GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, USER_DATA_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_CODE_READABLE | GDT_ACCESS_CODE_EXE_IN_RING | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, USER_CODE_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_DATA_WRITEABLE | GDT_ACCESS_DATA_DIRECTION_UP | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, UNUSED32_INDEX, 0, 0, 0, 0);
    gdt_set_entry(table, USER_CODE32_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_CODE_READABLE | GDT_ACCESS_CODE_EXE_IN_RING | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, USER_DATA32_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_DATA_WRITEABLE | GDT_ACCESS_DATA_DIRECTION_UP | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
#else
    gdt_set_entry(table, KERNEL_CODE_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_CODE_READABLE | GDT_ACCESS_CODE_EXE_IN_RING | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_RING0 | GDT_ACCESS_PRESENT, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, KERNEL_DATA_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_DATA_WRITEABLE | GDT_ACCESS_DATA_DIRECTION_UP | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_RING0 | GDT_ACCESS_PRESENT, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, USER_DATA_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_CODE_READABLE | GDT_ACCESS_CODE_EXE_IN_RING | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, USER_CODE_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_DATA_WRITEABLE | GDT_ACCESS_DATA_DIRECTION_UP | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
#endif
}

gdt_entry_t *gdt_get_entry(uint16_t sel)
{
    return NULL;
}

// debug function

void gdt_dump_selector(uint16_t sel)
{
#ifdef DEBUG

#endif
}

void x86_gdt_dump_selector_index(uint16_t index)
{
#ifdef DEBUG

#endif
}
