/*
 * File: gdt.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "gdt.h"
#include "VFS/vfs.h"
#include "libs/stdio.h"
#include "libs/memory.h"
#include "debug/debug.h"

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

#ifdef __i686__
void ASMCALL32 GDT_load_32(GDT_descriptor *descriptor);
#else
void GDT_load_64(GDT_descriptor *descriptor);
#endif

GDT_entry GDT_table[NUM_DESCRIPTOR];
GDT_descriptor GDT_descript = {sizeof(GDT_table) - 1, GDT_table};
tss_entry_t tss_entry;
extern char stack_top;

void x86_GDT_set_entry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    GDT_table[index] = (GDT_entry)GDT_ENTRY(base, limit, access, flags);
}

void x86_GDT_load()
{
    GDT_descript.Limit = sizeof(GDT_table) - 1;
    GDT_descript.Ptr = GDT_table;

#ifdef __x86_64__
    GDT_load_64(&GDT_descript);
#else
    GDT_load_32(&GDT_descript);
#endif

    log_debug(MODULE, "loading TSS entry");
    __asm__ volatile("ltr ax" : : "a"(x86_TSS_SEGMENT));
    log_debug(MODULE, "loaded TSS entry");
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
}

void x86_TSS_initialize()
{
    memset(&tss_entry, 0, sizeof(tss_entry_t));

#ifdef __i686__
    tss_entry.ss0 = x86_KERNEL_DATA_SEGMENT; // Set the kernel stack segment.
    tss_entry.sp0 = (uint32_t)&stack_top;   // Set the kernel stack segment.

    tss_entry.cs = x86_KERNEL_CODE_SEGMENT;
    tss_entry.ds = x86_KERNEL_DATA_SEGMENT;
    tss_entry.es = x86_KERNEL_DATA_SEGMENT;
    tss_entry.fs = x86_KERNEL_DATA_SEGMENT;
    tss_entry.gs = x86_KERNEL_DATA_SEGMENT;
    x86_GDT_set_entry(7, (uint32_t)&tss_entry, sizeof(tss_entry_t) - 1, 0x89, 0x40);
#else
    tss_entry.sp0 = (uint64_t)&stack_top;

    uint64_t base = (uint64_t)&tss_entry;
    uint16_t limit = sizeof(tss_entry_t) - 1;

    TSS_descriptor *td = (TSS_descriptor*)&GDT_table[7];
    td->LimitLow     = limit & 0xFFFF;
    td->BaseLow      = base & 0xFFFF;
    td->BaseMid      = (base >> 16) & 0xFF;
    td->Access       = 0x89;  // present, ring0, 64-bit TSS available
    td->FlagsLimitHi = (limit >> 16) & 0x0F;  // no G, no D/B, no L
    td->BaseHigh     = (base >> 24) & 0xFF;
    td->BaseUpper    = (base >> 32) & 0xFFFFFFFF;
    td->Reserved     = 0;

#endif
    log_debug(MODULE, "set TSS entry");
}

// debug function
#ifdef DEBUG

void x86_GDT_dump_selector(uint16_t sel)
{
    GDT_descriptor gdtr;
    __asm__("sgdt %0" : "=m"(gdtr));
    uint8_t *gdt = (uint8_t *)gdtr.Ptr;
    uint16_t index = sel >> 3;
    fprintf(stddebug, "selector 0x%04X index %u RPL %u", sel, index, sel & 3);
    // Print raw descriptor bytes
    uint8_t *desc = gdt + index * 8;
    fprintf(stddebug, "gdt[%u] = %02x %02x %02x %02x %02x %02x %02x %02x",
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
    fd_t file = VFS_open("/boot/bare.txt");
    uint8_t buf[255];
    VFS_read(file, buf, 255);
    VFS_close(file);
    if (memcmp(buf, "This file is required for booting.", 34))
    {
        panic("", "", 0x46, "what did the file say?");
    }
    _continue_to_user_mode = true;
    return true;
}
