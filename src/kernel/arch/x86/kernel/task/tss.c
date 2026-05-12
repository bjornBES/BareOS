/*
 * File: tss.c
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tss.h"
#include "libs/memory.h"
#include "debug/debug.h"

#define MODULE "TSS"

tss_t tss_entry;

void tss_load(uint16_t segment_sel)
{
    log_debug(MODULE, "loading TSS entry");
    __asm__ volatile("ltr ax" : : "a"(segment_sel));
    log_debug(MODULE, "loaded TSS entry");
}

void tss_set_kernel_sp(reg_t stack_pointer)
{
    tss_entry.sp0 = stack_pointer;
}

void tss_initialize(tss_t *tss, gdt_entry_t *descriptor)
{
    memset(tss, 0, sizeof(tss_t));

#ifdef __i686__
    tss->ss0 = x86_KERNEL_DATA_SEGMENT; // Set the kernel stack segment.
    tss->sp0 = (uint32_t)&stack_top;   // Set the kernel stack segment.

    tss->cs = x86_KERNEL_CODE_SEGMENT;
    tss->ds = x86_KERNEL_DATA_SEGMENT;
    tss->es = x86_KERNEL_DATA_SEGMENT;
    tss->fs = x86_KERNEL_DATA_SEGMENT;
    tss->gs = x86_KERNEL_DATA_SEGMENT;
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
