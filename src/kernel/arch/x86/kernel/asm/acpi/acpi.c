/*
 * File: ACPI.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi.h"
#include "acpi/table.h"
#include <boot/bootparams.h>
#include "kernel.h"
#include "kernel/mmu.h"

#define MODULE "ACPI"

void acpi_arch_init(boot_params_t *bp)
{
    rsdp_t *rsdp = (rsdp_t*)bp->acpi.rsdp_address;
    paddr_t addr = PAGE_ALIGN_DOWN((paddr_t)rsdp);
    mmu_arch_map(&kernel_page, (vaddr_t)addr, (paddr_t)addr, kernel_data_flags);
    log_info(MODULE, "%u,%u,%08x", rsdp->checksum, rsdp->revision, rsdp->rsdt_address);

    if (rsdp->revision != 0)
    {
        log_crit(MODULE, "rsdp is XSDT");
        for (;;)
        {
            ;
        }

        return;
    }

    paddr_t rsdt_address = (paddr_t)rsdp->rsdt_address;
    table_set_rsdt(rsdt_address);
}
