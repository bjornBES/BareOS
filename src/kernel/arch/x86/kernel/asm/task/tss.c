/*
 * File: tss.c
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tss.h"
#include "kernel/memory.h"
#include "kernel/cpu.h"
#include "debug/debug.h"

#define MODULE "TSS"

tss_t tss_entry;

void tss_load(uint16_t segment_sel)
{
    log_debug(MODULE, "loading TSS entry");
    __asm__ volatile("ltr ax" : : "a"(segment_sel));
    log_debug(MODULE, "loaded TSS entry");
}

void cpu_arch_set_kernel_stack(vaddr_t stack_top)
{
    tss_entry.sp0 = stack_top;
}

void tss_initialize(tss_t *tss, gdt_entry_t *descriptor)
{
    memset(tss, 0, sizeof(tss_t));

#ifdef __i686__
    tss->ss0 = KERNEL_DATA_SELECTOR; // Set the kernel stack segment.
    tss->sp0 = (uint32_t)&stack_top;   // Set the kernel stack segment.

    tss->cs = KERNEL_CODE_SELECTOR;
    tss->ds = KERNEL_DATA_SELECTOR;
    tss->es = KERNEL_DATA_SELECTOR;
    tss->fs = KERNEL_DATA_SELECTOR;
    tss->gs = KERNEL_DATA_SELECTOR;
    x86_GDT_set_entry(7, (uint32_t)tss, sizeof(tss_t) - 1, 0x89, 0x40);
#else

    uint64_t base = (uint64_t)tss;
    uint16_t limit = sizeof(tss_t) - 1;

    TSS_descriptor *td = (TSS_descriptor*)descriptor;
    memset(td, 0, sizeof(TSS_descriptor));
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

// limit = 103 + 0 + 0
// Base = 64 + 240 + 3 + 128
// Access = 139
// Flags = 0
// 103 0 64 240 3 139 0 128
