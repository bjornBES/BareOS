/*
 * File: table.c
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/table.h"

#include "asm/mmu_arch.h"
#include "asm/page_arch.h"

#include "debug/debug.h"

#include "mm/ioremap.h"

#include <config.h>

#define MODULE "ACPI-TABLE"

typedef struct
{
    sdt_header_t header;
    uint32_t entries[];
} PACKED rsdt_t;

typedef struct
{
    sdt_header_t header;
    uint64_t entries[];
} PACKED xsdt_t;

sdt_header_t *tables[CONFIG_ACPI_MAX_TABLES] = {0};
uint32_t signature_table[CONFIG_ACPI_MAX_TABLES] = {0};
int table_count = 0;

sdt_header_t *table_get_table(uint32_t signature)
{
    for (int i = 0; i < table_count; i++)
    {
        log_info(MODULE, "comparing 0x%04x to 0x%04x", signature_table[i], signature);
        if (signature_table[i] == signature)
        {
            return tables[i];
        }
    }
    return NULL;
}

int table_verify_checksum(sdt_header_t *header)
{
    uint8_t sum = 0;

    for (int i = 0; i < header->length; i++)
    {
        sum += ((uint8_t *)header)[i];
    }

    if (sum == 0)
    {
        return 0;
    }
    return 1;
}

void table_cache_tables(rsdt_t *rsd_table)
{
    int entries = (rsd_table->header.length - sizeof(sdt_header_t)) / 4;
    log_info(MODULE, "%u,%u", rsd_table->header.length, rsd_table->header.creator_revision);
    log_info(MODULE, "number of entries = %u", entries);
    for (int i = 0; i < entries; i++)
    {
        sdt_header_t *entry = (void *)(paddr_t)rsd_table->entries[i];

        if (table_count < CONFIG_ACPI_MAX_TABLES)
        {
            mmu_arch_map(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)entry), PAGE_ALIGN_DOWN((paddr_t)entry), kernel_data_flags);
            log_info(MODULE, "table[%i] sig=%04x/%c%c%c%c addr=%p", i, entry->signature, ((char*)&entry->signature)[0], ((char*)&entry->signature)[1], ((char*)&entry->signature)[2], ((char*)&entry->signature)[3], entry);
            signature_table[table_count] = entry->signature;

            uint32_t size = entry->length;
            mmu_arch_unmap(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)entry));
            tables[table_count++] = (sdt_header_t *)(ioremap((paddr_t)entry, size) + GET_PAGE_OFFSET((paddr_t)entry));
        }
        
    }
}

void table_set_base(vaddr_t base)
{
    mmu_arch_map(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)base), PAGE_ALIGN_DOWN((paddr_t)base), kernel_data_flags);
    rsdt_t *rsd_table = (rsdt_t *)base;
    uint32_t size = rsd_table->header.length;
    log_debug(MODULE, "size = %u", size);
    mmu_arch_unmap(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)base));
    rsd_table = (rsdt_t *)(ioremap((paddr_t)base, size) + GET_PAGE_OFFSET(base));
    log_debug(MODULE, "offset = %x", GET_PAGE_OFFSET(base));
    size = rsd_table->header.length;
    log_debug(MODULE, "size = %u", size);
    table_cache_tables(rsd_table);
}
