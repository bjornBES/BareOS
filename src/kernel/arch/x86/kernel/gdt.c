/*
 * File: gdt.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "gdt.h"
#include "VFS/vfs.h"
#include "libs/stdio.h"
#include "libs/memory.h"
#include "debug/debug.h"
#include "task/tss.h"

#define MODULE "GDT"

// helper functions
#define GDT_LIMIT_LOW(limit) (limit & 0xFFFF)
#define GDT_BASE_LOW(base) (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base) ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags) (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base) ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    access,                                     \
    GDT_FLAGS_LIMIT_HI(limit, flags),           \
    GDT_BASE_HIGH(base)}


gdt_entry_t gdt_table[GDT_ENTRIES];
gdtr_t gdt_descriptor = {sizeof(gdt_table) - 1, gdt_table};
extern char stack_top;

void x86_GDT_set_entry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    gdt_table[index] = (gdt_entry_t)GDT_ENTRY(base, limit, access, flags);
}

void x86_GDT_load(gdtr_t *gdt, gdt_entry_t* table)
{
    gdt->Limit = sizeof(gdt_entry_t[GDT_ENTRIES]) - 1;
    gdt->Ptr = table;

#ifdef __x86_64__
    gdt_load_64(gdt);
#else
    GDT_load_32(gdt);
#endif
}

void x86_GDT_initialize()
{
    x86_GDT_set_entry(0, 0, 0, 0, 0);
#ifdef __i686__
    x86_GDT_set_entry(1, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(2, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(3, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(4, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
#else
    x86_GDT_set_entry(1, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(2, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(3, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(4, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_64BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(5, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    x86_GDT_set_entry(6, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
#endif

    tss_initialize(&tss_entry, &gdt_table[7]);
}

// debug function
#ifdef DEBUG

void x86_GDT_dump_selector(uint16_t sel)
{
    gdtr_t gdtr;
    __asm__("sgdt %0" : "=m"(gdtr));
    uint8_t *gdt = (uint8_t *)gdtr.Ptr;
    uint16_t index = sel >> 3;
    fprintf(stddebug, "selector 0x%04X index %u RPL %u\n", sel, index, sel & 3);
    // Print raw descriptor bytes
    uint8_t *desc = gdt + index * 8;
    fprintf(stddebug, "gdt[%u] = %02x %02x %02x %02x %02x %02x %02x %02x\n",
            index, desc[0], desc[1], desc[2], desc[3], desc[4], desc[5], desc[6], desc[7]);
}

#endif

bool _continue_to_user_mode = false;
bool __unreachable()
{
    if (_continue_to_user_mode == true)
    {
        return true;
    }
    fd_t file = vfs_open("/boot/bare.txt", 0, 0);
    uint8_t buf[255];
    vfs_read(file, buf, 255);
    vfs_close(file);
    if (memcmp(buf, "This file is required for booting.", 34))
    {
        panic("", "", 0x46, "what did the file say?");
    }
    _continue_to_user_mode = true;
    return true;
}
