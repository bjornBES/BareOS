/*
 * File: gdt.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include "libs/memory.h"
#include <core/Defs.h>

#define x86_KERNEL_CODE_SEGMENT 0x08
#define x86_KERNEL_DATA_SEGMENT 0x10
#define x86_USER_CODE_SEGMENT 0x18
#define x86_USER_DATA_SEGMENT 0x20
#define x86_TSS_SEGMENT 0x28

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
} __attribute__((packed)) GDT_entry;

typedef struct
{
    uint16_t Limit; // sizeof(gdt) - 1
    GDT_entry *Ptr;  // address of GDT
} __attribute__((packed)) GDT_descriptor;

#ifdef __i686__
typedef struct tss_entry
{
    uint32_t prev_tss; // Previous TSS (not used)
    uint32_t esp0;     // Stack pointer for ring 0 (ISR stack)
    uint32_t ss0;      // Stack segment for ring 0
    uint32_t esp1;     // Stack pointer for ring 1 (not used)
    uint32_t ss1;      // Stack segment for ring 1 (not used)
    uint32_t esp2;     // Stack pointer for ring 2 (not used)
    uint32_t ss2;      // Stack segment for ring 2 (not used)
    uint32_t cr3;      // Page directory base
    uint32_t eip;      // Instruction pointer (not used here)
    uint32_t eflags;   // Flags (not used here)
    uint32_t eax;      // General-purpose registers (not used here)
    uint32_t ecx;      // General-purpose registers (not used here)
    uint32_t edx;      // General-purpose registers (not used here)
    uint32_t ebx;      // General-purpose registers (not used here)
    uint32_t esp;      // Stack pointer (not used here)
    uint32_t ebp;      // Base pointer (not used here)
    uint32_t esi;      // Source index (not used here)
    uint32_t edi;      // Destination index (not used here)
    uint32_t es;       // Data segment
    uint32_t cs;       // Code segment
    uint32_t ss;       // Stack segment
    uint32_t ds;       // Data segment
    uint32_t fs;       // Additional segment
    uint32_t gs;       // Additional segment
    uint32_t ldt;      // Local descriptor table (not used here)
    uint16_t trap;     // Trap flag (not used here)
    uint16_t iobase;   // I/O base (not used here)
} __attribute__((packed)) tss_entry_t;
#else
typedef struct tss_entry
{
    uint64_t iopb;
    uint64_t res4;
    uint64_t res3;
    uint64_t ist7;
    uint64_t ist6;
    uint64_t ist5;
    uint64_t ist4;
    uint64_t ist3;
    uint64_t ist2;
    uint64_t ist1;
    uint64_t res2;
    uint64_t rsp2;
    uint64_t rsp1;
    uint64_t rsp0;
    uint32_t res1;
} __attribute__((packed)) tss_entry_t;
#endif

#define NUM_DESCRIPTOR 8

extern tss_entry_t tss_entry;

void x86_GDT_load();
void x86_GDT_set_entry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void x86_GDT_initialize();

void x86_TSS_initialize();

// debug function
#ifdef DEBUG

void x86_GDT_dump_selector(uint16_t sel);

#endif
