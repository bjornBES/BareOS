/*
 * File: madt.c
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/madt/madt.h"

#include "asm/madt_arch.h"

#include "debug/debug.h"

#define MODULE "MADT"

int madt_parse()
{
    madt_t *_madt = (madt_t *)table_get_table(0x43495041); // apic
    if (!_madt)
    {
        log_crit(MODULE, "no madt");
        return -1;
    }

    return madt_arch_parse(_madt);
}
