/*
 * File: madt.c
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "madt.h"
#include "acpi/table.h"
#include "kernel/madt.h"

#define MODULE "madt"

void madt_parse()
{
    log_debug(MODULE, "got to here");
    madt *_madt = (madt *)table_get_table(0x41504943); // apic
    if (!_madt)
    {
        log_crit(MODULE, "no madt");
        return;
    }

    madt_arch_parse(_madt);
}
