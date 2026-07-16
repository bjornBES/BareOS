/*
 * File: cpuid.h
 * File Created: 09 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <defs.h>
#include "kernel/ctx.h"

typedef struct cpuid_regs {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_regs_t;

static inline void cpuid(reg_t leaf, reg_t sub_leaf, cpuid_regs_t *output)
{
    inline_asm("cpuid"  : "=a"(output->eax), "=b"(output->ebx), "=c"(output->ecx), "=d"(output->edx)
                        : "a" (leaf), "c" (sub_leaf)
                        :);
}
