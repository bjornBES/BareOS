/*
 * File: ACPI.c
 * File Created: 11 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ACPI.h"
#include "string.h"
#include "memory.h"
#include "bootparams/equipment/CPUID.h"
#include "x86.h"

#include <stddef.h>
#include <util/binary.h>

void DetectACPI(boot_params* bp)
{
    cpuid_regs reg;

    INIT_CPUID_REG(&reg);

    CPUID(1, 0, &reg);

    bp->acpi.lapicId = (reg.ebx >> 24) & 0xFF;

    for (size_t i = 0xE0000; i < 0xFFFFF; i++)
    {
        if (i == 0xFFFFF)
        {
            break;
        }
        char *addr = (char*)i;
        if (memcmp(addr, "RSD PTR ", 8) == 0)
        {
            bp->acpi.rsdpAddress = (uint32_t)((void*)addr);
            break;
        }
    }
    
}