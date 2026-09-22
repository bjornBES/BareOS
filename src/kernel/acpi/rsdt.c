/*
 * File: rsdt.c
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/rsdt.h"
#include "acpi/table.h"

#include "asm/page_arch.h"

#include "debug/debug.h"

#include "mm/ioremap.h"

// structure for revision 0 (version 1.0)
typedef struct rsdp
{
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} PACKED rsdp_t;

// structure for revision 2 (version 2.0+)
typedef struct xsdp
{
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address; // deprecated since version 2.0

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} PACKED xsdp_t;

#define MODULE "RSDT"

extern void hexdump(void *ptr, size_t len, size_t size);

status_t rsdt_parse(boot_params_t *bp)
{
    rsdp_t *rsdp = (rsdp_t *)(ioremap(bp->acpi.rsdp_address, 1024) + GET_PAGE_OFFSET(bp->acpi.rsdp_address));
    hexdump(rsdp, sizeof(rsdp_t), 16);

    trace_info(MODULE, "%u,%u,0x%08x", rsdp->checksum, rsdp->revision, rsdp->rsdt_address);

    if (rsdp->revision != 0)
    {
        log_crit(MODULE, "rsdp is XSDT");
        for (;;)
        {
            ;
        }

        return 1;
    }

    table_set_base((vaddr_t)rsdp->rsdt_address);

    return KERRNO_SUCCESSES;
}

