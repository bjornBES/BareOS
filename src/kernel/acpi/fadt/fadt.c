/*
 * File: fadt.c
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/fadt/fadt.h"
#include "acpi/dsdt/dsdt.h"
#include "acpi/table.h"

#include "debug/debug.h"

#define MODULE "fadt"

fadt_t *fadt = NULL;

extern void hexdump(void *ptr, size_t len, size_t size);

void fadt_parse()
{
    fadt = (fadt_t *)table_get_table(0x50434146);
    if (!fadt)
    {
        log_crit(MODULE, "no fadt_t found");
        return;
    }
    trace_info(MODULE, "fadt_t found revision=%u", fadt->h.revision);

    table_verify_checksum(&fadt->h);

    trace_debug(MODULE, "fadt->dsdt = %p", fadt->dsdt);
    trace_debug(MODULE, "fadt->x_dsdt = %p", fadt->x_dsdt);

    // paddr_t dsdt_phys = fadt->dsdt;
    // dsdt_init(dsdt_phys);
}
