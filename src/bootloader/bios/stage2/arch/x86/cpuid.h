/*
 * File: cpuid.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <defs.h>

#define INIT_CPUID_REG(regs) memset(regs, 0, sizeof(cpuid_regs))

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_regs;

INLINE SECTION("arch_text") void cpuid(uint32_t leaf, uint32_t sub_leaf, cpuid_regs* outregs)
{
    inline_asm("cpuid"
                : "=a"(outregs->eax), "=b"(outregs->ebx), "=c"(outregs->ecx), "=d"(outregs->edx)
                : "a"(leaf), "c"(sub_leaf));
}
