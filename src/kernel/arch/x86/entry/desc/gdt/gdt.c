/*
 * File: gdt.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "entry/desc/gdt/gdt.h"

#include "debug/debug.h"

#include "string.h"

#include <binary.h>
#include <defs.h>
#include <config.h>

// helper functions
#define GDT_LIMIT_LOW(limit)                  (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                    (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)                 ((base >> 16) & 0xFF)
#define GDT_LIMIT_HIGH(limit)                 ((limit >> 16) & 0xF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)      (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                   ((base >> 24) & 0xFF)
#define GDT_BASE_UPPER(base)                  ((base >> 32) & 0xFFFFFFFF)

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

void gdt_set_entry_system_segment(gdt_entry_t *table, uint16_t index, uint32_64_t base, uint32_t limit, uint8_t access, uint8_t flags)
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
    gdt_set_entry(table, USER_DATA_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_DATA_WRITEABLE | GDT_ACCESS_DATA_DIRECTION_UP | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K);
    gdt_set_entry(table, USER_CODE_INDEX, 0, 0xFFFFFFFF, GDT_ACCESS_CODE_READABLE | GDT_ACCESS_CODE_EXE_IN_RING | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_RING3 | GDT_ACCESS_PRESENT, GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K);
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
    gdtr_t gdtr;
    __asm__("sgdt %0" : "=m"(gdtr));
    uint8_t *gdt = (uint8_t *)gdtr.ptr;
    uint16_t index = sel >> 3;
    // Print raw descriptor bytes
    uint8_t *desc = gdt + index * 8;
    return (gdt_entry_t *)desc;
}

// debug function

void gdt_dump_selector(uint16_t sel)
{
#ifdef DEBUG
    uint16_t index = sel >> 3;
    x86_gdt_dump_selector_index(index);
#endif
}

void x86_gdt_dump_selector_index(uint16_t index)
{
#ifdef DEBUG
    gdtr_t gdtr;
    __asm__("sgdt %0" : "=m"(gdtr));
    gdt_entry_t *gdt = gdtr.ptr;
    // Print raw descriptor bytes
    gdt_entry_t *desc = &gdt[index];
    // 0x28
    // hexdump(gdt, sizeof(gdt_entry_t) * GDT_ENTRIES, 8);
    // hexdump(desc, sizeof(gdt_entry_t), 16);

    size_t limit = (desc->limit_high << 16) | desc->limit_low;
    paddr_t base = (desc->base_high << 24) | (desc->base_middle << 16) | desc->base_low;
    bool is_present = BIT_GET(desc->access, 7) == 1;
    int dpl = BIT_GET_RANGE(desc->access, 5, 6);
    char *desc_size = NULL;

    if (BIT_GET(desc->flags, 1) == 1)
    {
        desc_size = "LONG";
    }
    else if (BIT_GET(desc->flags, 2) == 1)
    {
        desc_size = "PMODE32";
    }
    else
    {
        desc_size = "PMODE16";
    }
    
    if (BIT_GET(desc->access, 4) == 0)
    {
        system_segment_descriptor_t *ss_desc = (system_segment_descriptor_t *)desc;
#ifdef CONFIG_64BIT
        base |= ((uint64_t)ss_desc->base_upper << 32);
#endif
        int desc_type = BIT_GET_RANGE(desc->access, 0, 3);
        trace_info(NO_MODULE, "gdt[%u] = system { base: %p, limit: %p, present: %s, ring%u, type: 0x%x, size: %s}", index, base, limit, is_present BOOL_TO_STRING, dpl, desc_type, desc_size);
        hexdump(ss_desc, sizeof(system_segment_descriptor_t), 16);
    }
    else
    {
        int accessed = BIT_GET(desc->access, 0);
        char *desc_type = NULL;
        if (BIT_GET(desc->access, 3) == 1)
        {
            desc_type = "TEXT";
        }
        else
        {
            desc_type = "DATA";
        }
        trace_info(NO_MODULE, "gdt[%u] = segment { base: %p, limit: %p, present: %s, ring%u, seg_type: %s, accessed: %s, size: %s}", index, base, limit, is_present BOOL_TO_STRING, dpl, desc_type, accessed BOOL_TO_STRING, desc_size);
        hexdump(desc, sizeof(gdt_entry_t), 16);
    }
#endif
}
