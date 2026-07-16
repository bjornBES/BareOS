/*
 * File: aml.c
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/aml/aml.h"

/* uint16_t aml_get_slp_typa()
{
    uint64_t dsdt_phys = (fadt->h.revision >= 2 && fadt->x_dsdt)
                             ? fadt->x_dsdt
                             : (uint64_t)fadt->dsdt;

    uint64_t addr = ALIGN_2_DOWN((uint32_64)dsdt_phys, PAGE_SIZE);
    paging_map_region(kernel_page, (virt_addr)addr, (phys_addr)addr, PAGE_SIZE, mmio_flags);

    sdt_header *dsdt = (sdt_header *)dsdt_phys;
    uint8_t *aml = (uint8_t *)dsdt + sizeof(sdt_header);
    uint32_t aml_len = dsdt->length - sizeof(sdt_header);
    paging_map_region(kernel_page, (virt_addr)addr, (phys_addr)addr, dsdt->length, mmio_flags);

    // scan for "_S5_" signature
    for (uint32_t i = 0; i < aml_len - 8; i++)
    {
        if (aml[i] == '_' && aml[i + 1] == 'S' &&
            aml[i + 2] == '5' && aml[i + 3] == '_')
        {
            return (uint16_t)aml[i + 7];
        }
    }

    log_warn(MODULE, "_S5 not found in DSDT, defaulting to 0");
    return 0;
} */

void aml_init(void *dsdt, uint32_t length)
{
    return;
}

int aml_eval(const char *path, aml_val_t *result)
{
    return 0;
}

void aml_notify(uint32_t event)
{
    return;
}
