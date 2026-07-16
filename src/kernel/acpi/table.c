/*
 * File: table.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "table.h"
#include "debug/debug.h"
#include "kernel/acpi/acpi.h"

#include "mm/mmu/mmu.h"
#include "mm/ioremap/ioremap.h"

#include "kernel/string.h"
#include "kernel/memory.h"

#define MODULE "ACPI-TABLE"

#define ACPI_MAX_TABLES 32

typedef struct signature
{
    char sig[5];
} __attribute__((packed)) signature_t;


sdt_header *tables[ACPI_MAX_TABLES] = {0};
signature_t signature_table[ACPI_MAX_TABLES] = {0};
int table_count = 0;
void *rsdt_base;

sdt_header *table_get_table(uint32_t signature)
{
    char cSignature[4] = {0};
    memcpy(cSignature, &signature, 4);
    uint64_t realSignature = ((uint64_t)cSignature[0] << 24) | ((uint64_t)cSignature[1] << 16) | ((uint64_t)cSignature[2] << 8) | ((uint64_t)cSignature[3]);
    for (int i = 0; i < table_count; i++)
    {
        log_info(MODULE, "comparing '%4s' to %4s", &signature_table[i], &realSignature);
        if (memcmp(&signature_table[i], &realSignature, 4) == 0)
        {
            return tables[i];
        }
    }
    return NULL;
}

bool table_verify_checksum(sdt_header *header)
{
    uint8_t sum = 0;

    for (int i = 0; i < header->length; i++)
    {
        sum += ((uint8_t *)header)[i];
    }

    return sum == 0;
}

void table_cache_tables()
{
    rsdt *rsd_table = (rsdt *)rsdt_base;
    int entries = (rsd_table->header.length - sizeof(sdt_header)) / 4;
    log_info(MODULE, "%u,%u", rsd_table->header.length, rsd_table->header.creator_revision);
    log_info(MODULE, "number of entries = %u", entries);
    for (size_t i = 0; i < entries; i++)
    {
        sdt_header *entry = (void *)(paddr_t)rsd_table->entries[i];

        if (table_count < ACPI_MAX_TABLES)
        {
            mmu_arch_map(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)entry), PAGE_ALIGN_DOWN((paddr_t)entry), kernel_data_flags);
            log_info(MODULE, "table[%i] sig=%c%c%c%c addr=%p", i, entry->signature[0], entry->signature[1], entry->signature[2], entry->signature[3], entry);
            memcpy(&signature_table[table_count], entry->signature, 4);
            signature_table[table_count].sig[4] = '\0';

            uint32_t size = entry->length;
            paddr_t offset = (paddr_t)entry & (PAGE_SIZE - 1);
            mmu_arch_unmap(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)entry));
            tables[table_count++] = (sdt_header *)(ioremap((paddr_t)entry, size) + offset);
        }
        
    }
}

void table_set_rsdt(paddr_t base)
{
    paddr_t offset = base & (PAGE_SIZE - 1);
    mmu_arch_map(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)base), PAGE_ALIGN_DOWN((paddr_t)base), kernel_data_flags);
    rsdt *rsd_table = (rsdt *)base;
    uint32_t size = rsd_table->header.length;
    log_debug(MODULE, "size = %u", size);
    mmu_arch_unmap(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)base));
    rsd_table = (rsdt *)(ioremap((paddr_t)base, size) + offset);
    log_debug(MODULE, "offset = %x", offset);
    size = rsd_table->header.length;
    log_debug(MODULE, "size = %u", size);
    rsdt_base = rsd_table;
    table_cache_tables();
}
