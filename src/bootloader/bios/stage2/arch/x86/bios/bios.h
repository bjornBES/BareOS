/*
 * File: bios.h
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "bios_regs.h"
#include "debug/debug.h"
#include <stdint.h>
#include <defs.h>

INLINE SECTION("arch_text") uint16_t bios_get_ds()
{
    uint16_t seg;
    __asm__("mov %0,ds" : "=r"(seg));
    return seg;
}

INLINE SECTION("arch_text") void bios_set_fs(uint16_t seg)
{
    __asm__("mov fs,%0" : : "r"(seg));
}

INLINE SECTION("arch_text") uint16_t bios_get_fs()
{
    uint16_t seg;
    __asm__("mov %0,fs" : "=r"(seg));
    return seg;
}

INLINE SECTION("arch_text") void bios_set_gs(uint16_t seg)
{
    __asm__("mov gs,%0" : : "r"(seg));
}

INLINE SECTION("arch_text") uint16_t bios_get_gs()
{
    uint16_t seg;
    __asm__("mov %0,gs" : "=r"(seg));
    return seg;
}

void bios_arch_dump_frame(bios_regs_t *frame);

void bios_arch_init_regs(bios_regs_t *regs);

ASMCALL32 extern void bios_intcall(uint8_t int_no, bios_regs_t *ireg, bios_regs_t *oreg);

void bios_arch_intcall(uint8_t int_no, bios_regs_t *ireg, bios_regs_t *oreg);
