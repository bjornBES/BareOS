/*
 * File: hpet.c
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/hpet/hpet.h"
#include "acpi/table.h"

#include "asm/hpet_arch.h"

#include "debug/debug.h"

#define MODULE "hpet"

void hpet_parse()
{
    sdt_header_t *hpet = (sdt_header_t *)table_get_table(0x54455048);
    if (!hpet)
    {
        log_crit(MODULE, "no hpet found");
        return;
    }
    trace_info(MODULE, "hpet found revision=%u", hpet->revision);

    table_verify_checksum(hpet);

    hpet_arch_init(hpet);
}
