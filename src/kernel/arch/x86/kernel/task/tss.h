/*
 * File: tss.h
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "../gdt.h"
#include <stdint.h>

#ifdef __i686__
typedef struct tss_entry
{
    uint32_t prev_tss; // Previous TSS (not used)
    uint32_t sp0;     // Stack pointer for ring 0 (ISR stack)
    uint32_t ss0;      // Stack segment for ring 0
    uint32_t sp1;     // Stack pointer for ring 1 (not used)
    uint32_t ss1;      // Stack segment for ring 1 (not used)
    uint32_t sp2;     // Stack pointer for ring 2 (not used)
    uint32_t ss2;      // Stack segment for ring 2 (not used)
    uint32_t cr3;      // Page directory base
    uint32_t pc;      // Instruction pointer (not used here)
    uint32_t flags;   // Flags (not used here)
    uint32_t ax;      // General-purpose registers (not used here)
    uint32_t cx;      // General-purpose registers (not used here)
    uint32_t dx;      // General-purpose registers (not used here)
    uint32_t bx;      // General-purpose registers (not used here)
    uint32_t sp;      // Stack pointer (not used here)
    uint32_t bp;      // Base pointer (not used here)
    uint32_t si;      // Source index (not used here)
    uint32_t di;      // Destination index (not used here)
    uint32_t es;       // Data segment
    uint32_t cs;       // Code segment
    uint32_t ss;       // Stack segment
    uint32_t ds;       // Data segment
    uint32_t fs;       // Additional segment
    uint32_t gs;       // Additional segment
    uint32_t ldt;      // Local descriptor table (not used here)
    uint16_t trap;     // Trap flag (not used here)
    uint16_t iobase;   // I/O base (not used here)
} __attribute__((packed)) tss_t;
#else
typedef struct {
    uint16_t LimitLow;
    uint16_t BaseLow;
    uint8_t  BaseMid;
    uint8_t  Access;      // 0x89 = present, ring0, TSS available
    uint8_t  FlagsLimitHi;
    uint8_t  BaseHigh;
    uint32_t BaseUpper;   // bits 63:32
    uint32_t Reserved;
} __attribute__((packed)) TSS_descriptor;
typedef struct tss_entry
{
    uint32_t res1;
    uint64_t sp0;
    uint64_t sp1;
    uint64_t sp2;
    uint64_t res2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t res3;
    uint16_t res4;
    uint16_t iopb;
} __attribute__((packed)) tss_t;
#endif

extern tss_t tss_entry;

void tss_load(uint16_t segment_sel);
void tss_set_kernel_sp(reg_t stack_pointer);
void tss_initialize(tss_t *tss, gdt_entry_t *descriptor);
