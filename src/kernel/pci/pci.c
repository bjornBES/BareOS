/*
 * File: pci.c
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pci/msi/pcidev_msi.h"
#include "pci/pci.h"
#include "pci/pci_regs.h"
#include "pci/mcfg.h"

#include "dev/device.h"

#include "asm/arch_pci.h"

#include "drivers/bust.h"
#include "drivers/bus.h"

#include "mm/ioremap.h"

#include "irq/irq.h"

#include "memory.h"
#include "string.h"

#include "kerrno.h"

#include "stdio.h"

#include <binary.h>

#define MODULE "pci"

size_t pci_device_count;
size_t pci_capacity;
pci_device_t **pci_devices;

status_t pci_config_read8(pci_device_t *dev, uint16_t offset, uint8_t *out)
{
    *out = pci_arch_read_config32(dev, offset) >> ((offset & 3) << 3);
    return KERRNO_SUCCESSES;
}

status_t pci_config_read16(pci_device_t *dev, uint16_t offset, uint16_t *out)
{
    *out = pci_arch_read_config32(dev, offset) >> ((offset & 2) << 3);
    return KERRNO_SUCCESSES;
}

status_t pci_config_read32(pci_device_t *dev, uint16_t offset, uint32_t *out)
{
    *out = pci_arch_read_config32(dev, offset);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write8(pci_device_t *dev, uint16_t offset, uint8_t val)
{
    uint32_t val32 = (uint32_t)val << ((offset & 3) << 3);
    uint32_t old_data = pci_arch_read_config32(dev, offset) & ~(0xFFu << ((offset & 3) << 3));
    uint32_t data = old_data | val32;
    pci_arch_write_config32(dev, offset, data);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write16(pci_device_t *dev, uint16_t offset, uint16_t val)
{
    uint32_t val32 = (uint32_t)val << ((offset & 2) << 3);
    uint32_t old_data = pci_arch_read_config32(dev, offset) & ~(0xFFFFu << ((offset & 2) << 3));
    uint32_t data = old_data | val32;
    pci_arch_write_config32(dev, offset, data);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write32(pci_device_t *dev, uint16_t offset, uint32_t val)
{
    pci_arch_write_config32(dev, offset, val);
    return KERRNO_SUCCESSES;
}

status_t pci_init_capabilities(pci_device_t *dev)
{
    ENTER_FUNC("%p", dev);
    // pci_ea_init(dev);		/* Enhanced Allocation */
    dev->no_msi = pci_msi_init(dev) != KERRNO_SUCCESSES;           /* Disable MSI */
    dev->msix_enabled = !(pci_msix_init(dev) == KERRNO_SUCCESSES); /* Disable MSI-X */

    /* Buffers for saving PCIe and PCI-X capabilities */
    // pci_allocate_cap_save_buffers(dev);

    // pci_imm_ready_init(dev);	/* Immediate Readiness */
    // pci_pm_init(dev);		/* Power Management */
    // pci_vpd_init(dev);		/* Vital Product Data */
    // pci_configure_ari(dev);		/* Alternative Routing-ID Forwarding */
    // pci_iov_init(dev);		/* Single Root I/O Virtualization */
    // pci_ats_init(dev);		/* Address Translation Services */
    // pci_pri_init(dev);		/* Page Request Interface */
    // pci_pasid_init(dev);		/* Process Address Space ID */
    // pci_acs_init(dev);		/* Access Control Services */
    // pci_ptm_init(dev);		/* Precision Time Measurement */
    // pci_aer_init(dev);		/* Advanced Error Reporting */
    // pci_dpc_init(dev);		/* Downstream Port Containment */
    // pci_rcec_init(dev);		/* Root Complex Event Collector */
    // pci_doe_init(dev);		/* Data Object Exchange */
    // pci_tph_init(dev);		/* TLP Processing Hints */
    // pci_rebar_init(dev);		/* Resizable BAR */
    // pci_dev3_init(dev);		/* Device 3 capabilities */
    // pci_ide_init(dev);		/* Link Integrity and Data Encryption */

    // pcie_report_downtraining(dev);
    // pci_init_reset_methods(dev);
    return KERRNO_SUCCESSES;
}

INTERNAL pci_bar_t pci_int_decode_bar(pci_device_t *dev, int offset, pci_bar_t *out_abar);

status_t pci_get_bar(pci_device_t *dev, uint8_t bar_index, pci_bar_t *out)
{
    ENTER_FUNC("%p, %u, %p", dev, bar_index, out);
    // pci_int_decode_bar(dev, bar_index * 4 + 0x10, &dev->bars[bar_index]);
    trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", dev->bars[bar_index].base, dev->bars[bar_index].is_64bit BOOL_TO_STRING, dev->bars[bar_index].is_mmio BOOL_TO_STRING, dev->bars[bar_index].prefetchable BOOL_TO_STRING, dev->bars[bar_index].size);
    memcpy(out, &dev->bars[bar_index], sizeof(pci_bar_t));
    return KERRNO_SUCCESSES;
}

status_t pci_map_bar(pci_device_t *dev, uint8_t bar_index, vaddr_t *out_vaddr)
{
    *out_vaddr = ioremap(dev->bars[bar_index].base, dev->bars[bar_index].size);
    return KERRNO_SUCCESSES;
}

status_t pci_find_capability(pci_device_t *dev, uint8_t cap_id, uint16_t *out_offset)
{
    uint16_t conf_id = dev->config_ptr;
    if (conf_id == 0x0000)
    {
        return KERRNO_SUCCESSES;
    }
    uint16_t conf = 0x0;
    do
    {
        pci_config_read16(dev, conf_id, &conf);
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

status_t pci_find_ext_capability(pci_device_t *dev, uint16_t cap_id, uint16_t *out_offset)
{
    return KERRNO_SUCCESSES;
}

status_t pci_get_irq(pci_device_t *dev, interrupt_vector_t *out_line, kernel_irq_t *out_pin)
{
    if (dev->msi_enabled)
    {
        pci_msi_register_irq(dev, out_line, out_pin);
    }
    return KERRNO_SUCCESSES;
}

status_t pci_enable_device(pci_device_t *dev)
{
    return KERRNO_SUCCESSES;
}

status_t pci_set_bus_master(pci_device_t *dev, uint8_t enable)
{
    uint16_t old_cmd;
    uint16_t cmd;

    status_t ret = pci_config_read16(dev, PCI_HEADER_COMMAND, &old_cmd);
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
        ret = pci_config_write16(dev, PCI_HEADER_COMMAND, cmd);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
    }
    dev->is_bus_master = enable;
    return KERRNO_SUCCESSES;
}

status_t pci_enable_interrupt(pci_device_t *dev)
{
    return KERRNO_SUCCESSES;
}

status_t pci_enable_dma(pci_device_t *dev)
{
    return KERRNO_SUCCESSES;
}

status_t pci_set_command_flags(pci_device_t *dev, pci_command_bits_t flags)
{

    return KERRNO_SUCCESSES;
}

status_t pci_clear_command_flags(pci_device_t *dev, pci_command_bits_t flags)
{
    return KERRNO_SUCCESSES;
}

INTERNAL pci_bar_t pci_int_decode_bar(pci_device_t *dev, int offset, pci_bar_t *out_abar)
{
    ENTER_FUNC("%p, 0x%x, %p", dev, offset, out_abar);
    uint32_t bar;
    pci_config_read32(dev, offset, &bar);

    pci_bar_t abar = {
        .is_mmio = (bar & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY,
    };
    if (abar.is_mmio)
    {
        abar.prefetchable = FLAG_IS_SET(bar, PCI_BASE_ADDRESS_MEM_PREFETCH);
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
                pci_config_read32(dev, offset + 4, &bar2);
                base = ((uint64_t)bar2 << 32) | (bar & PCI_BASE_ADDRESS_MEM_MASK);
                break;

            default :
                break;
        }
        abar.base = base;
        uint32_t size = ~0;
        pci_config_write32(dev, offset, size);
        pci_config_read32(dev, offset, &size);

        abar.size = (~size) + 1;
        pci_config_write32(dev, offset, bar);
    }
    
    pci_config_write32(dev, offset, bar);
    trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", abar.base, abar.is_64bit BOOL_TO_STRING, abar.is_mmio BOOL_TO_STRING, abar.prefetchable BOOL_TO_STRING, abar.size);
    memcpy(out_abar, &abar, sizeof(pci_bar_t));
    return abar;
}

bool pci_check_bus(uint32_t bus, uint32_t slot, uint8_t func, pci_device_t *pdev)
{
    pdev->bdf.bus = bus;
    pdev->bdf.device = slot;
    pdev->bdf.function = func;
    uint16_t vendor;
    pci_config_read16(pdev, PCI_HEADER_VENDOR_ID, &vendor);
    if (vendor == 0xFFFF)
    {
        return false;
    }

    {
        pdev->vendor_id = vendor;
        pci_config_read16(pdev, PCI_HEADER_DEVICE_ID, &pdev->device_id);

        pci_config_read16(pdev, PCI_HEADER_COMMAND, &pdev->command);
        pci_config_read16(pdev, PCI_HEADER_STATUS, &pdev->status);

        uint32_t reg2Data;
        pci_config_read32(pdev, PCI_HEADER_REVISION_ID, &reg2Data);
        pdev->class_code = (reg2Data >> 24) & 0xFF;
        pdev->sub_class = (reg2Data >> 16) & 0xFF;
        pdev->prog_if = (reg2Data >> 8) & 0xFF;
        pdev->revision = (reg2Data) & 0xFF;

        uint32_t reg3Data;
        pci_config_read32(pdev, PCI_HEADER_CACHE_LINE_SIZE, &reg3Data);
        pdev->bist = (reg3Data >> 24) & 0xFF;
        pdev->header_type = (reg3Data >> 16) & 0xFF;
        pdev->latency_timer = (reg3Data >> 8) & 0xFF;
        pdev->cache_line_size = (reg3Data) & 0xFF;
    }

    pdev->multi_function = (pdev->header_type & PCI_HEADER_TYPE_MFD) == PCI_HEADER_TYPE_MFD;
    pdev->header_type &= PCI_HEADER_TYPE_MASK;

    {
        if (pdev->header_type == PCI_HEADER_TYPE_CARDBUS)
        {
            trace_debug(MODULE, "header_type = card bus");
            pci_config_read8(pdev, PCI_HEADER_CB_CAPABILITY_LIST, &pdev->config_ptr);
        }
        else
        {
            trace_debug(MODULE, "header_type = 0 or 1");
            pci_config_read8(pdev, PCI_HEADER_CAPABILITY_LIST, &pdev->config_ptr);
        }

        if (pdev->header_type == PCI_HEADER_TYPE_NORMAL)
        {
            pci_int_decode_bar(pdev, PCI_HEADER_BASE_ADDRESS_0, &pdev->bars[0]);
            pci_int_decode_bar(pdev, PCI_HEADER_BASE_ADDRESS_1, &pdev->bars[1]);
            pci_int_decode_bar(pdev, PCI_HEADER_BASE_ADDRESS_2, &pdev->bars[2]);
            pci_int_decode_bar(pdev, PCI_HEADER_BASE_ADDRESS_3, &pdev->bars[3]);
            pci_int_decode_bar(pdev, PCI_HEADER_BASE_ADDRESS_4, &pdev->bars[4]);
            pci_int_decode_bar(pdev, PCI_HEADER_BASE_ADDRESS_5, &pdev->bars[5]);

            /*             pdev->header.header0.card_bus_cis = pci_config_read_dword(bus, slot, func, 0x28);
                        pdev->header.header0.subsystem_id = pci_config_read_word(bus, slot, func, 0x2C);
                        pdev->header.header0.subsystem_vendor_id = pci_config_read_word(bus, slot, func, 0x2E);
                        pdev->header.header0.rom_base_address = pci_config_read_dword(bus, slot, func, 0x30);
                        pdev->header.header0.capabilities_ptr = pci_config_read_dword(bus, slot, func, 0x34) & 0xFF;
                        uint32_t reg3Data = pci_config_read_dword(bus, slot, func, 0x3C);
                        pdev->header.header0.max_latency = (reg3Data >> 24) & 0xFF;
                        pdev->header.header0.min_grant = (reg3Data >> 16) & 0xFF;
                        pdev->header.header0.interrupt_pin = (reg3Data >> 8) & 0xFF;
                        pdev->header.header0.interrupt_line = (reg3Data) & 0xFF; */
        }
    }

    pci_init_capabilities(pdev);

    return true;
}

extern bus_type_t pci_bus_type;

void pci_check_buses()
{
    for (uint32_t bus = 0; bus < 256; bus++)
    {
        for (uint32_t slot = 0; slot < 32; slot++)
        {
            uint16_t function = 0;
            {
                pci_device_t pdev;
                bool result = pci_check_bus(bus, slot, function, &pdev);
                if (result == false)
                {
                    continue;
                }

                pci_devices[pci_device_count] = kmalloc(sizeof(pci_device_t));
                memcpy(pci_devices[pci_device_count], &pdev, sizeof(pci_device_t));
            }
            {
                pci_device_t *pdev = pci_devices[pci_device_count];

                // trace_debug(MODULE, "pci_devices[%u] = %p", pci_device_count, &pdev);
                device_t *dev = &pdev->dev;
                memset(dev, 0, sizeof(*dev));
                pci_device_id_t *id = &pdev->id;
                id->vendor = pdev->vendor_id;
                id->device = pdev->device_id;
                id->class = (pdev->class_code << 16) | (pdev->sub_class << 8) | pdev->prog_if;
                dev->bus = &pci_bus_type;
                trace_debug(MODULE, "pdev @ %p, dev @ %p", pdev, dev);
                device_register(dev);
                pci_device_count++;

                // trace_debug(MODULE, "vendor: 0x%x device: 0x%x", pdev->vendor_id, pdev->device_id);
                trace_debug(MODULE, "bus: 0x%x, slot: 0x%x, function: 0x%x", pdev->bdf.bus, pdev->bdf.device, pdev->bdf.function);
                trace_debug(MODULE, "device_id: 0x%x, vendor: 0x%x", pdev->device_id, pdev->vendor_id);
                trace_debug(MODULE, "status: 0x%x, command: 0x%x", pdev->status, pdev->command);
                trace_debug(MODULE, "class_code: 0x%x, sub_class: 0x%x, prog_if: 0x%x, revision: 0x%x", pdev->class_code, pdev->sub_class, pdev->prog_if, pdev->revision);
                trace_debug(MODULE, "bist: 0x%x, header_type: 0x%x, latency_timer: 0x%x, cache_line_size: 0x%x", pdev->bist, pdev->header_type, pdev->latency_timer, pdev->cache_line_size);
                log_debug(NO_MODULE, "");
                if (pdev->multi_function)
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
