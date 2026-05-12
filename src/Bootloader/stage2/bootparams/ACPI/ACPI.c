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
#include "stdio.h"

#include <stddef.h>
#include <util/binary.h>

extern void hexdump(void *ptr, int len);

void DetectACPI(boot_params *bp)
{
    cpuid_regs reg;

    INIT_CPUID_REG(&reg);

    CPUID(1, 0, &reg);

    bp->acpi.lapicId = (reg.ebx >> 24) & 0xFF;
    bp->acpi.rsdpAddress = 0;

    uint16_t ebda_segment = *(uint16_t *)0x40E;
    uint32_t ebda_phys = (uint32_t)ebda_segment << 4; // multiply by 16

    for (size_t i = 0; i < 0x400; i += 16)
    {
        char *addr = (char *)(i + ebda_phys);
        // hexdump(addr, 16);
        if (memcmp(addr, "RSD PTR ", 8) == 0)
        {
            printf("found %p", addr);
            bp->acpi.rsdpAddress = (uint32_t)((void *)addr);
            break;
        }
    }

    if (bp->acpi.rsdpAddress != 0)
    {
        return;
    }

    for (size_t i = 0xE0000; i < 0xFFFFF; i += 16)
    {
        if (i >= 0xFFFFF)
        {
            break;
        }
        char *addr = (char *)i;
        // hexdump(addr, 16);
        if (memcmp(addr, "RSD PTR ", 8) == 0)
        {
            printf("found %p", addr);
            bp->acpi.rsdpAddress = (uint32_t)((void *)addr);
            break;
        }
    }
}