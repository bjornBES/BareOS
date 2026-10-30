/*
 * File: pci.c
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/pci/msi/pcidev_msi.h"
#include "drivers/pci/pci.h"
#include "drivers/pci/pci_regs.h"
#include "drivers/pci/mcfg.h"

#include "dev/device.h"

#include "asm/arch_pci.h"

#include "drivers/base/class.h"
#include "drivers/base/bust.h"
#include "drivers/base/bus.h"

#include "mm/ioremap.h"

#include "resource/resource.h"

#include "irq/irq.h"

#include "memory.h"
#include "string.h"
#include "math.h"

#include "kerrno.h"

#include "stdio.h"

#include "align.h"

#include <binary.h>

#define MODULE "pci"

size_t pci_device_count;
size_t pci_capacity;
pci_device_t **pci_devices;

status_t pci_config_read8(pci_device_t *pci_dev, uint16_t offset, uint8_t *out)
{
    *out = pci_arch_read_config32(pci_dev, offset) >> ((offset & 3) << 3);
    return KERRNO_SUCCESSES;
}

status_t pci_config_read16(pci_device_t *pci_dev, uint16_t offset, uint16_t *out)
{
    *out = pci_arch_read_config32(pci_dev, offset) >> ((offset & 2) << 3);
    return KERRNO_SUCCESSES;
}

status_t pci_config_read32(pci_device_t *pci_dev, uint16_t offset, uint32_t *out)
{
    *out = pci_arch_read_config32(pci_dev, offset);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write8(pci_device_t *pci_dev, uint16_t offset, uint8_t val)
{
    uint32_t val32 = (uint32_t)val << ((offset & 3) << 3);
    uint32_t old_data = pci_arch_read_config32(pci_dev, offset) & ~(0xFFu << ((offset & 3) << 3));
    uint32_t data = old_data | val32;
    pci_arch_write_config32(pci_dev, offset, data);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write16(pci_device_t *pci_dev, uint16_t offset, uint16_t val)
{
    uint32_t val32 = (uint32_t)val << ((offset & 2) << 3);
    uint32_t old_data = pci_arch_read_config32(pci_dev, offset) & ~(0xFFFFu << ((offset & 2) << 3));
    uint32_t data = old_data | val32;
    pci_arch_write_config32(pci_dev, offset, data);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write32(pci_device_t *pci_dev, uint16_t offset, uint32_t val)
{
    pci_arch_write_config32(pci_dev, offset, val);
    return KERRNO_SUCCESSES;
}

status_t pci_init_capabilities(pci_device_t *pci_dev)
{
    ENTER_FUNC("%p", pci_dev);
    // pci_ea_init(pci_dev);		/* Enhanced Allocation */
    pci_dev->no_msi = pci_msi_init(pci_dev) != KERRNO_SUCCESSES;           /* Disable MSI */
    pci_dev->msix_enabled = !(pci_msix_init(pci_dev) == KERRNO_SUCCESSES); /* Disable MSI-X */

    /* Buffers for saving PCIe and PCI-X capabilities */
    // pci_allocate_cap_save_buffers(pci_dev);

    // pci_imm_ready_init(pci_dev);	/* Immediate Readiness */
    // pci_pm_init(pci_dev);		/* Power Management */
    // pci_vpd_init(pci_dev);		/* Vital Product Data */
    // pci_configure_ari(pci_dev);		/* Alternative Routing-ID Forwarding */
    // pci_iov_init(pci_dev);		/* Single Root I/O Virtualization */
    // pci_ats_init(pci_dev);		/* Address Translation Services */
    // pci_pri_init(pci_dev);		/* Page Request Interface */
    // pci_pasid_init(pci_dev);		/* Process Address Space ID */
    // pci_acs_init(pci_dev);		/* Access Control Services */
    // pci_ptm_init(pci_dev);		/* Precision Time Measurement */
    // pci_aer_init(pci_dev);		/* Advanced Error Reporting */
    // pci_dpc_init(pci_dev);		/* Downstream Port Containment */
    // pci_rcec_init(pci_dev);		/* Root Complex Event Collector */
    // pci_doe_init(pci_dev);		/* Data Object Exchange */
    // pci_tph_init(pci_dev);		/* TLP Processing Hints */
    // pci_rebar_init(pci_dev);		/* Resizable BAR */
    // pci_dev3_init(pci_dev);		/* Device 3 capabilities */
    // pci_ide_init(pci_dev);		/* Link Integrity and Data Encryption */

    // pcie_report_downtraining(pci_dev);
    // pci_init_reset_methods(pci_dev);
    return KERRNO_SUCCESSES;
}

INTERNAL pci_bar_t pci_int_decode_bar(pci_device_t *pci_dev, int offset, int bar_index, pci_bar_t *out_abar);

status_t pci_get_bar(pci_device_t *pci_dev, uint8_t bar_index, pci_bar_t *out)
{
    ENTER_FUNC("%p, %u, %p", pci_dev, bar_index, out);
    pci_int_decode_bar(pci_dev, bar_index * 4 + 0x10, bar_index, &pci_dev->bars[bar_index]);
    trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", pci_dev->bars[bar_index].base, pci_dev->bars[bar_index].is_64bit BOOL_TO_STRING, pci_dev->bars[bar_index].is_mmio BOOL_TO_STRING, pci_dev->bars[bar_index].prefetchable BOOL_TO_STRING, pci_dev->bars[bar_index].size);
    memcpy(out, &pci_dev->bars[bar_index], sizeof(pci_bar_t));
    return KERRNO_SUCCESSES;
}

status_t pci_map_bar(pci_device_t *pci_dev, uint8_t bar_index, vaddr_t *out_vaddr)
{
    resource_t *res = &pci_dev->resource[bar_index];
    if (res->space != RES_SPACE_MEM)
    {
        KERRNO_RETURN(KERRNO_DONT_DO_THAT, "can only map memory");
    }
    *out_vaddr = ioremap(pci_dev->bars[bar_index].base, pci_dev->bars[bar_index].size);
    return KERRNO_SUCCESSES;
}

status_t pci_find_capability(pci_device_t *pci_dev, uint8_t cap_id, uint16_t *out_offset)
{
    uint16_t conf_id = pci_dev->config_ptr;
    if (conf_id == 0x0000)
    {
        return KERRNO_SUCCESSES;
    }
    uint16_t conf = 0x0;
    do
    {
        pci_config_read16(pci_dev, conf_id, &conf);
        if (conf == 0)
        {
            return KERRNO_UNSUCCESS;
        }
        trace_debug(MODULE, "conf_id = 0x%x, conf = 0x%x", conf_id, conf);

        uint8_t id = (uint8_t)(conf & PCI_CAP_ID_MASK);
        if (id == 0xFF)
        {
            return KERRNO_UNSUCCESS;
        }
        if (id == cap_id)
        {
            *out_offset = conf_id;
            break;
        }
        conf_id = ((conf & PCI_CAP_LIST_NEXT_MASK) >> 8);
    } while (conf_id != 0);
    return KERRNO_SUCCESSES;
}

status_t pci_find_ext_capability(pci_device_t *pci_dev, uint16_t cap_id, uint16_t *out_offset)
{
    return KERRNO_SUCCESSES;
}

status_t pci_get_irq(pci_device_t *pci_dev, interrupt_vector_t *out_line, kernel_irq_t *out_pin)
{
    if (pci_dev->msi_enabled)
    {
        pci_msi_register_irq(pci_dev, out_line, out_pin);
    }
    return KERRNO_SUCCESSES;
}

status_t pci_enable_device(pci_device_t *pci_dev)
{
    return KERRNO_SUCCESSES;
}

status_t pci_set_bus_master(pci_device_t *pci_dev, uint8_t enable)
{
    uint16_t old_cmd;
    uint16_t cmd;

    status_t ret = pci_config_read16(pci_dev, PCI_HEADER_COMMAND, &old_cmd);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    if (enable)
    {
        cmd = old_cmd | PCI_COMMAND_MASTER;
    }
    else
    {
        cmd = old_cmd & ~PCI_COMMAND_MASTER;
    }

    if (cmd != old_cmd)
    {
        ret = pci_config_write16(pci_dev, PCI_HEADER_COMMAND, cmd);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
    }
    pci_dev->is_bus_master = enable;
    return KERRNO_SUCCESSES;
}

status_t pci_enable_interrupt(pci_device_t *pci_dev)
{
    return KERRNO_SUCCESSES;
}

status_t pci_enable_dma(pci_device_t *pci_dev)
{
    return KERRNO_SUCCESSES;
}

status_t pci_set_command_flags(pci_device_t *pci_dev, pci_command_bits_t flags)
{

    return KERRNO_SUCCESSES;
}

status_t pci_clear_command_flags(pci_device_t *pci_dev, pci_command_bits_t flags)
{
    return KERRNO_SUCCESSES;
}

INTERNAL pci_bar_t pci_int_decode_bar(pci_device_t *pci_dev, int offset, int bar_index, pci_bar_t *out_abar)
{
    ENTER_FUNC("%p, 0x%x, %p", pci_dev, offset, out_abar);
    resource_t *bar_res = &pci_dev->resource[bar_index];
    res_flags_t flags = 0;
    uint32_t bar;
    pci_config_read32(pci_dev, offset, &bar);

    pci_bar_t abar = {
        .is_mmio = (bar & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY,
    };
    if (abar.is_mmio)
    {
        abar.prefetchable = FLAG_IS_SET(bar, PCI_BASE_ADDRESS_MEM_PREFETCH);
        if (abar.prefetchable)
        {
            flags |= RES_FLAG_PREFETCH;
        }
        uint32_t type = bar & PCI_BASE_ADDRESS_MEM_TYPE_MASK;
        paddr_t base;
        switch (type)
        {
            case PCI_BASE_ADDRESS_MEM_TYPE_32 :
                abar.is_64bit = 0;
                base = bar & PCI_BASE_ADDRESS_MEM_MASK;
                break;
            case PCI_BASE_ADDRESS_MEM_TYPE_1M :
                abar.is_64bit = 0;
                base = bar & 0xFFF0;
                break;
            case PCI_BASE_ADDRESS_MEM_TYPE_64 :
                abar.is_64bit = 1;
                uint32_t bar2 = 0;
                pci_config_read32(pci_dev, offset + 4, &bar2);
                base = ((uint64_t)bar2 << 32) | (bar & PCI_BASE_ADDRESS_MEM_MASK);
                break;

            default :
                break;
        }
        abar.base = base;
        uint32_t size = 0;
        pci_config_write32(pci_dev, offset, (uint32_t)PCI_BASE_ADDRESS_MEM_MASK);
        pci_config_read32(pci_dev, offset, &size);

        abar.size = PAGE_ALIGN_UP((~size) + 1);
        pci_config_write32(pci_dev, offset, bar);
    }
    else
    {
        paddr_t base = bar & PCI_BASE_ADDRESS_IO_MASK;
        abar.base = base;
        uint32_t size = 0;
        pci_config_write32(pci_dev, offset, (uint32_t)PCI_BASE_ADDRESS_IO_MASK);
        pci_config_read32(pci_dev, offset, &size);

        abar.size = PAGE_ALIGN_UP((~size) + 1);
        pci_config_write32(pci_dev, offset, bar);
    }

    pci_config_write32(pci_dev, offset, bar);
    trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", abar.base, abar.is_64bit BOOL_TO_STRING, abar.is_mmio BOOL_TO_STRING, abar.prefetchable BOOL_TO_STRING, abar.size);
    // char *res_name = kzmalloc(PCI_MAX_NAME);
    // size_t length = sprintf(res_name, "%04x:%04x.%02x.%02x.%x.%x", dev->vendor_id, dev->device_id, dev->bdf.bus, dev->bdf.device, dev->bdf.function, bar_index);
    // res_name[length] = '\0';
    memcpy(out_abar, &abar, sizeof(pci_bar_t));
    return abar;
}

bool pci_check_bus(uint32_t bus, uint32_t slot, uint8_t func, pci_device_t *pci_dev)
{
    pci_dev->bdf.bus = bus;
    pci_dev->bdf.device = slot;
    pci_dev->bdf.function = func;
    uint16_t vendor;
    pci_config_read16(pci_dev, PCI_HEADER_VENDOR_ID, &vendor);
    if (vendor == 0xFFFF)
    {
        return false;
    }

    {
        pci_dev->vendor_id = vendor;
        pci_config_read16(pci_dev, PCI_HEADER_DEVICE_ID, &pci_dev->device_id);

        pci_config_read16(pci_dev, PCI_HEADER_COMMAND, &pci_dev->command);
        pci_config_read16(pci_dev, PCI_HEADER_STATUS, &pci_dev->status);

        uint32_t reg2Data;
        pci_config_read32(pci_dev, PCI_HEADER_REVISION_ID, &reg2Data);
        pci_dev->class_code = (reg2Data >> 24) & 0xFF;
        pci_dev->sub_class = (reg2Data >> 16) & 0xFF;
        pci_dev->prog_if = (reg2Data >> 8) & 0xFF;
        pci_dev->revision = (reg2Data) & 0xFF;

        uint32_t reg3Data;
        pci_config_read32(pci_dev, PCI_HEADER_CACHE_LINE_SIZE, &reg3Data);
        pci_dev->bist = (reg3Data >> 24) & 0xFF;
        pci_dev->header_type = (reg3Data >> 16) & 0xFF;
        pci_dev->latency_timer = (reg3Data >> 8) & 0xFF;
        pci_dev->cache_line_size = (reg3Data) & 0xFF;
    }

    char *res_name = kzmalloc(PCI_MAX_NAME);
    size_t length = sprintf(res_name, "%04x:%04x.%02x.%02x.%x", pci_dev->vendor_id, pci_dev->device_id, bus, slot, func);
    res_name[length] = '\0';
    resource_t *dev_slot_mem = resource_create();
    dev_slot_mem->name = res_name;
    dev_slot_mem->type = RES_TYPE_PCI_BAR;
    dev_slot_mem->start = ~0;
    resource_t *dev_slot_io = resource_create();
    dev_slot_io->name = res_name;
    dev_slot_io->type = RES_TYPE_PCI_BAR;
    dev_slot_io->start = ~0;

    pci_dev->multi_function = (pci_dev->header_type & PCI_HEADER_TYPE_MFD) == PCI_HEADER_TYPE_MFD;
    pci_dev->header_type &= PCI_HEADER_TYPE_MASK;

    {
        if (pci_dev->header_type == PCI_HEADER_TYPE_CARDBUS)
        {
            trace_debug(MODULE, "header_type = card bus");
            pci_config_read8(pci_dev, PCI_HEADER_CB_CAPABILITY_LIST, &pci_dev->config_ptr);
        }
        else
        {
            trace_debug(MODULE, "header_type = 0 or 1");
            pci_config_read8(pci_dev, PCI_HEADER_CAPABILITY_LIST, &pci_dev->config_ptr);
        }

        if (pci_dev->header_type == PCI_HEADER_TYPE_NORMAL)
        {
            pci_int_decode_bar(pci_dev, PCI_HEADER_BASE_ADDRESS_0, 0, &pci_dev->bars[0]);
            pci_int_decode_bar(pci_dev, PCI_HEADER_BASE_ADDRESS_1, 1, &pci_dev->bars[1]);
            pci_int_decode_bar(pci_dev, PCI_HEADER_BASE_ADDRESS_2, 2, &pci_dev->bars[2]);
            pci_int_decode_bar(pci_dev, PCI_HEADER_BASE_ADDRESS_3, 3, &pci_dev->bars[3]);
            pci_int_decode_bar(pci_dev, PCI_HEADER_BASE_ADDRESS_4, 4, &pci_dev->bars[4]);
            pci_int_decode_bar(pci_dev, PCI_HEADER_BASE_ADDRESS_5, 5, &pci_dev->bars[5]);
            for (size_t i = 0; i < 6; i++)
            {
                pci_bar_t *abar = &pci_dev->bars[i];
                if (abar->size != 0)
                {
                    if (abar->is_mmio)
                    {
                        if (abar->prefetchable)
                        {
                            dev_slot_mem->flags |= RES_FLAG_PREFETCH;
                        }
                        dev_slot_mem->start = min(dev_slot_mem->start, abar->base);
                        dev_slot_mem->end = max(dev_slot_mem->end, abar->base + abar->size);
                    }
                    else
                    {
                        dev_slot_io->start = min(dev_slot_io->start, abar->base);
                        dev_slot_io->end = max(dev_slot_io->end, abar->base + abar->size);
                    }
                }
            }
            if (dev_slot_mem->start != ~0)
            {
                resource_insert(&mem_space.root, dev_slot_mem);
            }
            else
            {
                kfree(dev_slot_mem);
            }
            if (dev_slot_io->start != ~0)
            {
                resource_insert(&io_space.root, dev_slot_io);
            }
            else
            {
                kfree(dev_slot_io);
            }

            pci_config_read16(pci_dev, PCI_HEADER_SUBSYSTEM_VENDOR_ID, (uint16_t *)&pci_dev->id.sub_vendor);
            pci_config_read16(pci_dev, PCI_HEADER_SUBSYSTEM_ID, (uint16_t *)&pci_dev->id.sub_device);

            uint32_t reg3Data = 0;
            pci_config_read32(pci_dev, PCI_HEADER_INTERRUPT_LINE, &reg3Data);
            pci_dev->irq_pin = (reg3Data >> 8) & 0xFF;
            pci_dev->irq_line = (reg3Data) & 0xFF;
            uint32_t rom;
            pci_config_read32(pci_dev, 0x30, &rom);
            if (rom & PCI_ROM_ADDRESS_ENABLE)
            {
                resource_t *rom_res = resource_create();
                uint32_t size = PCI_ROM_ADDRESS_MASK;
                pci_config_write32(pci_dev, PCI_HEADER_ROM_ADDRESS, size);
                pci_config_read32(pci_dev, PCI_HEADER_ROM_ADDRESS, &size);

                pci_dev->romlen = (~size) + 1;
                pci_config_write32(pci_dev, PCI_HEADER_ROM_ADDRESS, rom);
                pci_dev->rom = rom & PCI_ROM_ADDRESS_MASK;

                rom_res->type = RES_TYPE_PCI_BUS;
                rom_res->flags = RES_FLAG_READONLY;
                rom_res->start = pci_dev->rom;
                rom_res->end = pci_dev->rom + pci_dev->romlen;
                rom_res->name = kzmalloc(PCI_MAX_NAME);
                size_t length = sprintf(rom_res->name, "%04x:%04x.%02x.%02x.%x", pci_dev->vendor_id, pci_dev->device_id, pci_dev->bdf.bus, pci_dev->bdf.device, pci_dev->bdf.function);
                rom_res->name[length] = '\0';
                resource_insert(&mem_space.root, rom_res);
            }

            /*             pci_dev->header.header0.card_bus_cis = pci_config_read_dword(bus, slot, func, 0x28);
                        pci_dev->header.header0.capabilities_ptr = pci_config_read_dword(bus, slot, func, 0x34) & 0xFF;
                        pci_dev->header.header0.max_latency = (reg3Data >> 24) & 0xFF;
                        pci_dev->header.header0.min_grant = (reg3Data >> 16) & 0xFF;
                        pci_dev->header.header0.interrupt_pin = (reg3Data >> 8) & 0xFF;
                        pci_dev->header.header0.interrupt_line = (reg3Data) & 0xFF; */
        }
    }

    pci_init_capabilities(pci_dev);

    return true;
}

extern bus_type_t pci_bus_type;

status_t pci_device_added(device_t *dev)
{
    return KERRNO_SUCCESSES;
}

static dev_class_t pcibus_class = {
    .name = "pci_bus",
    .type = DEVICE_PCI,
    .device_added = pci_device_added,
};

void pci_check_buses()
{
    for (uint32_t bus = 0; bus < 256; bus++)
    {
        for (uint32_t slot = 0; slot < 32; slot++)
        {
            uint16_t function = 0;
            pci_device_t *pci_dev;
            {
                pci_dev = kzmalloc(sizeof(pci_device_t));
                bool result = pci_check_bus(bus, slot, function, pci_dev);
                if (result == false)
                {
                    kfree(pci_dev);
                    continue;
                }

                pci_devices[pci_device_count] = pci_dev;
            }
            {
                log_debug(NO_MODULE, "");

                // trace_debug(MODULE, "pci_devices[%u] = %p", pci_device_count, &pdev);
                device_t *dev = &pci_dev->dev;
                trace_debug(MODULE, "here0 dev->flags = 0x%x", dev->flags);
                dev_set_ready_to_probe(dev);
                trace_debug(MODULE, "here1 dev->flags = 0x%x", dev->flags);
                dev->class = &pcibus_class;
                trace_debug(MODULE, "here2 dev->flags = 0x%x", dev->flags);
                pci_device_id_t *id = &pci_dev->id;
                id->vendor = pci_dev->vendor_id;
                id->device = pci_dev->device_id;
                id->class = (pci_dev->class_code << 16) | (pci_dev->sub_class << 8) | pci_dev->prog_if;
                trace_debug(MODULE, "here3 dev->flags = 0x%x", dev->flags);
                dev->bus = &pci_bus_type;
                trace_debug(MODULE, "here4 dev->flags = 0x%x", dev->flags);
                trace_debug(MODULE, "pci_dev @ %p, dev @ %p", pci_dev, dev);
                // trace_debug(MODULE, "vendor: 0x%x device: 0x%x", pci_dev->vendor_id, pci_dev->device_id);
                trace_debug(MODULE, "bus: 0x%x, slot: 0x%x, function: 0x%x", pci_dev->bdf.bus, pci_dev->bdf.device, pci_dev->bdf.function);
                trace_debug(MODULE, "device_id: 0x%x, vendor: 0x%x", pci_dev->device_id, pci_dev->vendor_id);
                trace_debug(MODULE, "status: 0x%x, command: 0x%x", pci_dev->status, pci_dev->command);
                trace_debug(MODULE, "class_code: 0x%x, sub_class: 0x%x, prog_if: 0x%x, revision: 0x%x", pci_dev->class_code, pci_dev->sub_class, pci_dev->prog_if, pci_dev->revision);
                trace_debug(MODULE, "bist: 0x%x, header_type: 0x%x, latency_timer: 0x%x, cache_line_size: 0x%x", pci_dev->bist, pci_dev->header_type, pci_dev->latency_timer, pci_dev->cache_line_size);
                device_register(dev);
                pci_device_count++;

                if (pci_dev->multi_function)
                {
                    function = 1;
                    for (; function < 8; function++)
                    {
                        pci_device_t pdev_func;
                        bool result = pci_check_bus(bus, slot, function, &pdev_func);
                        if (result == false)
                        {
                            continue;
                        }
                        pci_devices[pci_device_count] = kmalloc(sizeof(pci_device_t));
                        memcpy(pci_devices[pci_device_count], &pdev_func, sizeof(pci_device_t));
                        // trace_debug(MODULE, "pci_devices[%u] = %p", pci_device_count, &pdev_func);
                        pci_device_count++;

                        // trace_debug(MODULE, "vendor: 0x%x device: 0x%x", pdev_func.vendor_id, pdev_func.device_id);
                        trace_debug(MODULE, "bus: 0x%x, slot: 0x%x, function: 0x%x", pdev_func.bdf.bus, pdev_func.bdf.device, pdev_func.bdf.function);
                        trace_debug(MODULE, "device_id: 0x%x, vendor: 0x%x", pdev_func.device_id, pdev_func.vendor_id);
                        trace_debug(MODULE, "status: 0x%x, command: 0x%x", pdev_func.status, pdev_func.command);
                        trace_debug(MODULE, "class_code: 0x%x, sub_class: 0x%x, prog_if: 0x%x, revision: 0x%x", pdev_func.class_code, pdev_func.sub_class, pdev_func.prog_if, pdev_func.revision);
                        trace_debug(MODULE, "bist: 0x%x, header_type: 0x%x, latency_timer: 0x%x, cache_line_size: 0x%x", pdev_func.bist, pdev_func.header_type, pdev_func.latency_timer, pdev_func.cache_line_size);
                        log_debug(NO_MODULE, "");
                    }
                }
                else
                {
                }
            }
        }
    }
}

status_t pci_register_driver(pci_driver_t *driver)
{
    driver->driver.bus = &pci_bus_type;
    driver->driver.name = driver->name;
    drivers_register(&driver->driver);
    return KERRNO_SUCCESSES;
}

status_t pci_unregister_driver()
{
    return KERRNO_SUCCESSES;
}

status_t pci_initialize()
{
    class_register(&pcibus_class);
    pci_device_count = 0;
    pci_capacity = 16;
    pci_devices = (pci_device_t **)kmalloc(sizeof(pci_device_t *) * 16);
    trace_debug(MODULE, "pci_devices = %p", pci_devices);
    if (mcfg_parse() != KERRNO_SUCCESSES)
    {
    }

    pci_check_buses();
    return KERRNO_SUCCESSES;
}
