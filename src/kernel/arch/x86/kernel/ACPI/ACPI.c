/*
 * File: ACPI.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ACPI.h"
#include "debug/debug.h"
#include "memory/paging/paging.h"

#include "libs/string.h"

#include <stdbool.h>

#define MODULE "ACPI"

#define ACPI_MAX_TABLES 32

static SDT_header *s_tables[ACPI_MAX_TABLES] = {0};
static int s_table_count = 0;

SDT_header *ACPI_get_table(const char *signature)
{
    for (int i = 0; i < s_table_count; i++)
    {
        log_info(MODULE, "comparing '%4s' with '%s'", s_tables[i]->signature, signature);
        if (strncmp(s_tables[i]->signature, signature, 4) == 0)
        {
            return s_tables[i];
        }
    }
    return NULL;
}

bool ACPI_verify_checksum(SDT_header *header)
{
    uint8_t sum = 0;

    for (int i = 0; i < header->length; i++)
    {
        sum += ((uint8_t *)header)[i];
    }

    return sum == 0;
}

void ACPI_init(ACPI_info info)
{
    RSDP *rsdp = (RSDP *)info.rsdpAddress;
    uint32_64 addr = ALIGN_2_DOWN((uint32_64)rsdp, PAGE_SIZE);
    paging_map_region(kernel_page, (virt_addr)addr, (phys_addr)addr, PAGE_SIZE, kernel_data_flags);
    log_info(MODULE, "%u,%u,%08x", rsdp->checksum, rsdp->revision, rsdp->rsdt_address);

    if (rsdp->revision != 0)
    {
        log_crit(MODULE, "rsdp is XSDT");
        for (size_t i = 0; i < 100000; i++)
        {
            continue;
        }
        return;
    }

    uint32_64 rsdt_phys = (uint32_64)rsdp->rsdt_address;
    addr = ALIGN_2_DOWN((uint32_64)rsdt_phys, PAGE_SIZE);

    paging_map_region(kernel_page, (virt_addr)addr, (phys_addr)addr, PAGE_SIZE + 1, kernel_data_flags);

    RSDT *rsdt = (RSDT *)rsdt_phys;
    paging_map_region(kernel_page, (virt_addr)addr, (phys_addr)addr, rsdt->header.length, kernel_data_flags);
    bool checksum = ACPI_verify_checksum(&rsdt->header);
    int entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
    log_info(MODULE, "%s,%u,%u,%u", checksum BOOT_TO_STRING, rsdt->header.length, rsdt->header.creator_id, rsdt->header.creator_revision);
    log_info(MODULE, "number of entries = %u", entries);
    for (size_t i = 0; i < entries; i++)
    {
        SDT_header *entry = (void *)(uint32_64)rsdt->entries[i];
        
        if (s_table_count < ACPI_MAX_TABLES)
        {
            s_tables[s_table_count++] = entry;
        }
        log_info(MODULE, "table[%i] sig=% 4s addr=%p", i, s_tables[i]->signature, s_tables[i]);
    }
}