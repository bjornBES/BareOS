/*
 * File: CPUID.c
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "CPUID.h"
#include "Equipment.h"
#include "x86.h"
#include "memory.h"
#include "stdio.h"

#include <util/binary.h>

#define BPCPUID bp->cpu

void DetectCPUID(boot_params* bp)
{
    memset(&bp->cpu, 0, sizeof(CPU_info));

    cpuid_regs reg;

    INIT_CPUID_REG(&reg);
    CPUID(0, 0, &reg);

    memcpy(BPCPUID.vendor, &reg.ebx, 4);
    memcpy(BPCPUID.vendor + 4, &reg.edx, 4);
    memcpy(BPCPUID.vendor + 8, &reg.ecx, 4);

    printf("CPUID %s\n", BPCPUID.vendor);
}