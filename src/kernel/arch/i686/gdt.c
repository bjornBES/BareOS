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
#include <stdio.h>
#include <memory.h>

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

void ASMCALL GDT_Load(GDTDescriptor *descriptor, uint16_t codeSegment, uint16_t dataSegment);

void ASMCALL i686_tss_load();

GDTEntry _GDT[NUM_DESCRIPTOR];
GDTDescriptor _GDTDescriptor = {sizeof(_GDT) - 1, _GDT};
tss_entry_t tss_entry;
extern char stack_top;

void i686_GDTSetEntry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    _GDT[index] = (GDTEntry)GDT_ENTRY(base, limit, access, flags);
}

void i686_GDTLoad()
{
    GDT_Load(&_GDTDescriptor, i686_KERNEL_CODE_SEGMENT, i686_KERNEL_DATA_SEGMENT);
}

void i686_GDTInitialize()
{
    i686_GDTSetEntry(0, 0, 0, 0, 0);
    i686_GDTSetEntry(1, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    i686_GDTSetEntry(2, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    i686_GDTSetEntry(3, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    i686_GDTSetEntry(4, 0, 0xFFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
}

void i686_tss_initialize()
{
    memset(&tss_entry, 0, sizeof(tss_entry_t));

    tss_entry.ss0 = i686_KERNEL_DATA_SEGMENT; // Set the kernel stack segment.
    tss_entry.esp0 = (uint32_t)&stack_top; // Set the kernel stack segment.

    tss_entry.cs = i686_KERNEL_CODE_SEGMENT;
    tss_entry.ds = i686_KERNEL_DATA_SEGMENT;
    tss_entry.es = i686_KERNEL_DATA_SEGMENT;
    tss_entry.fs = i686_KERNEL_DATA_SEGMENT;
    tss_entry.gs = i686_KERNEL_DATA_SEGMENT;

    i686_GDTSetEntry(5, (uint32_t)&tss_entry, sizeof(tss_entry_t) - 1, 0x89, 0x00);

    i686_tss_load();
}

// debug function
#ifdef DEBUG

void i686_GDT_DumpSelector(uint16_t sel)
{
    GDTDescriptor gdtr;
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
