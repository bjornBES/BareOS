/*
 * File: acpi.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi.h"
#include "memory.h"

void acpi_detect(boot_params_t *bt)
{
    bt->acpi.rsdp_address = 0;
    uint16_t ebda_segment = *(uint16_t *)0x40E;
    uint32_t ebda_phys = (uint32_t)ebda_segment << 4; // multiply by 16

    for (size_t i = 0; i < 0x400; i += 16)
    {
        char *addr = (char *)(i + ebda_phys);
        // hexdump(addr, 16);
        if (memcmp(addr, "RSD PTR ", 8) == 0)
        {
            bt->acpi.rsdp_address = (uint32_t)((void *)addr);
            break;
        }
    }

    if (bt->acpi.rsdp_address != 0)
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
            bt->acpi.rsdp_address = (uint32_t)((void *)addr);
            break;
        }
    }
}
