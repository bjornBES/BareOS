/*
 * File: MSR.c
 * File Created: 06 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "MSR.h"
#include "kernel/asm/cpuid/cpuid.h"
#include "kernel.h"
#include <util/binary.h>
#include <defs.h>

int msr_has_msr()
{
    cpuid_regs_t regs;
    cpuid(1, 0, &regs);
    if (BIT_GET(regs.edx, 5))
    {
        return RETURN_GOOD;
    }
    return RETURN_FAILED;
}
