/*
 * File: tss.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tss.h"

#include "memory.h"

#include <defs.h>

void tss_load(uint16_t segment_sel)
{
    inline_asm("ltr ax" : : "a"(segment_sel));
}

void tss_initialize(tss_entry_t *tss, gdt_entry_t *table, int tss_index)
{
    memset(tss, 0, sizeof(tss_entry_t));

#ifdef __i686__
    tss->ss0 = KERNEL_DATA_SELECTOR; // Set the kernel stack segment.
    tss->sp0 = (uint32_t)&stack_top; // Set the kernel stack segment.

    tss->cs = KERNEL_CODE_SELECTOR;
    tss->ds = KERNEL_DATA_SELECTOR;
    tss->es = KERNEL_DATA_SELECTOR;
    tss->fs = KERNEL_DATA_SELECTOR;
    tss->gs = KERNEL_DATA_SELECTOR;
    x86_GDT_set_entry(7, (uint32_t)tss, sizeof(tss_t) - 1, 0x89, 0x40);
#else

    uint64_t base = (uint64_t)tss;
    uint16_t limit = sizeof(tss_entry_t) - 1;

    gdt_set_entry_system_segment(table, tss_index, base, limit, 0x89, 0);
#endif
}
