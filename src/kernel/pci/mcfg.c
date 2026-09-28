/*
 * File: mcfg.c
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pci/mcfg.h"

#include "debug/debug.h"

#include "mm/ioremap.h"

#include "asm/mmu_arch.h"
#include "mm/mmu.h"

#include "pci/pci.h"

#include "kerrno.h"

#define MODULE "mcfg"

mcfg_t *mcfg = NULL;

extern void hexdump(void *ptr, size_t len, size_t size);

status_t mcfg_parse()
{
    mcfg = (mcfg_t *)table_get_table(0x4746434D);
    if (!mcfg)
    {
        log_crit(MODULE, "no mcfg found");
        return 1;
    }

    hexdump(mcfg, mcfg->h.length, 16);
    trace_debug(MODULE, "mcfg->reserved = %p", mcfg->reserved);
    trace_debug(MODULE, "mcfg->entries = %p", mcfg->entries);
    trace_debug(MODULE, "&mcfg->entries = %p", mcfg->entries);
    hexdump(&mcfg->entries, sizeof(mcfg_entry_t), 16);
    mcfg_entry_t *entries = mcfg->entries;

    mmu_map_region(&kernel_page, entries[0].addr, entries[0].addr, 65536 * PAGE_SIZE, mmio_flags);
    for (uint32_t bus = 0; bus < 256; bus++)
    {
        for (uint32_t slot = 0; slot < 32; slot++)
        {
            paddr_t addr = (((bus * 256) + (slot * 8)) * 0x1000) + entries[0].addr;
            pci_device_t *entry = (pci_device_t *)(addr - offsetof(pci_device_t, vendor_id));
            if (entry->vendor_id != 0xFFFF)
            {
                hexdump((void *)entry + offsetof(pci_device_t, vendor_id), sizeof(pci_device_t), 32);
                trace_debug(MODULE, "bus: 0x%x, slot: 0x%x, function: 0", bus, slot);
                trace_debug(MODULE, "device_id: 0x%x, vendor: 0x%x", entry->device_id, entry->vendor_id);
                trace_debug(MODULE, "status: 0x%x, command: 0x%x", entry->status, entry->command);
                trace_debug(MODULE, "class_code: 0x%x, sub_class: 0x%x, prog_if: 0x%x, revision: 0x%x", entry->class_code, entry->sub_class, entry->prog_if, entry->revision);
                trace_debug(MODULE, "bist: 0x%x, header_type: 0x%x, latency_timer: 0x%x, cache_line_size: 0x%x", entry->bist, entry->header_type, entry->latency_timer, entry->cache_line_size);
            }
        }
        // 0x4142
        // 0x4552
    }
    mmu_free_region(&kernel_page, entries[0].addr, 65536 * PAGE_SIZE);



    return KERRNO_SUCCESSES;
}
