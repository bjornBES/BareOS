/*
 * File: pci.c
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pci/pci.h"
#include "pci/pci_regs.h"
#include "pci/mcfg.h"

#include "dev/device.h"

#include "asm/arch_pci.h"

#include "drivers/bust.h"
#include "drivers/bus.h"

#include "memory.h"
#include "string.h"

#include "kerrno.h"

#include "stdio.h"

#include <binary.h>

#define MODULE "pci"

size_t pci_device_count;
size_t pci_capacity;
pci_device_t **pci_devices;

uint8_t pci_config_read8(pci_device_t *dev, uint16_t offset)
{
    return pci_arch_read_config32(dev, offset) >> ((offset & 3) << 3);
}

uint16_t pci_config_read16(pci_device_t *dev, uint16_t offset)
{
    return pci_arch_read_config32(dev, offset) >> ((offset & 2) << 3);
}

uint32_t pci_config_read32(pci_device_t *dev, uint16_t offset)
{
    return pci_arch_read_config32(dev, offset);
}

status_t pci_config_write8(pci_device_t *dev, uint16_t offset, uint8_t val)
{
    uint32_t val32 = (uint32_t)val << ((offset & 3) << 3);
    uint32_t old_data = pci_config_read32(dev, offset) & ~(0xFFu << ((offset & 3) << 3));
    uint32_t data = old_data | val32;
    pci_arch_write_config32(dev, offset, data);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write16(pci_device_t *dev, uint16_t offset, uint16_t val)
{
    uint32_t val32 = (uint32_t)val << ((offset & 2) << 3);
    uint32_t old_data = pci_config_read32(dev, offset) & ~(0xFFFFu << ((offset & 2) << 3));
    uint32_t data = old_data | val32;
    pci_arch_write_config32(dev, offset, data);
    return KERRNO_SUCCESSES;
}

status_t pci_config_write32(pci_device_t *dev, uint16_t offset, uint32_t val)
{
    pci_arch_write_config32(dev, offset, val);
    return KERRNO_SUCCESSES;
}

status_t pci_get_bar(pci_device_t *dev, uint8_t bar_index, pci_bar_t *out)
{
    memcpy(out, &dev->bars[bar_index], sizeof(pci_bar_t));
    return KERRNO_SUCCESSES;
}

status_t pci_map_bar(pci_device_t *dev, uint8_t bar_index, void *out_vaddr)
{
    return KERRNO_SUCCESSES;
}

status_t pci_find_capability(pci_device_t *dev, uint8_t cap_id, uint16_t *out_offset)
{
    return KERRNO_SUCCESSES;
}

status_t pci_find_ext_capability(pci_device_t *dev, uint16_t cap_id, uint16_t *out_offset)
{
    return KERRNO_SUCCESSES;
}

status_t pci_get_irq(pci_device_t *dev, interrupt_vector_t *out_line, uint8_t *out_pin)
{
    return KERRNO_SUCCESSES;
}

status_t pci_enable_device(pci_device_t *dev)
{
    return KERRNO_SUCCESSES;
}

status_t pci_set_bus_master(pci_device_t *dev, uint8_t enable)
{
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

INTERNAL pci_bar_t pci_int_decode_bar(pci_device_t *dev, uint32_t bar, int offset, pci_bar_t *out_abar)
{
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
                uint32_t bar2 = pci_config_read32(dev, offset + 4);
                base = ((uint64_t)bar2 << 32) | (bar & PCI_BASE_ADDRESS_MEM_MASK);
                break;

            default :
                break;
        }
        abar.base = base;
        pci_config_write32(dev, offset, ~0);
        uint32_t size = pci_config_read32(dev, offset);
        
        abar.size = (~size) + 1;
        pci_config_write32(dev, offset, bar);
    }
    memcpy(out_abar, &abar, sizeof(pci_bar_t));
    return abar;
}

bool pci_check_bus(uint32_t bus, uint32_t slot, uint8_t func, pci_device_t *pdev)
{
    pdev->bdf.bus = bus;
    pdev->bdf.device = slot;
    pdev->bdf.function = func;
    uint16_t vendor = pci_config_read16(pdev, PCI_HEADER_VENDOR_ID);
    if (vendor == 0xFFFF)
    {
        return false;
    }

    {
        pdev->vendor_id = vendor;
        pdev->device_id = pci_config_read16(pdev, PCI_HEADER_DEVICE_ID);

        pdev->command = pci_config_read16(pdev, PCI_HEADER_COMMAND);
        pdev->status = pci_config_read16(pdev, PCI_HEADER_STATUS);

        uint32_t reg2Data = pci_config_read32(pdev, PCI_HEADER_REVISION_ID);
        pdev->class_code = (reg2Data >> 24) & 0xFF;
        pdev->sub_class = (reg2Data >> 16) & 0xFF;
        pdev->prog_if = (reg2Data >> 8) & 0xFF;
        pdev->revision = (reg2Data) & 0xFF;

        uint32_t reg3Data = pci_config_read32(pdev, PCI_HEADER_CACHE_LINE_SIZE);
        pdev->bist = (reg3Data >> 24) & 0xFF;
        pdev->header_type = (reg3Data >> 16) & 0xFF;
        pdev->latency_timer = (reg3Data >> 8) & 0xFF;
        pdev->cache_line_size = (reg3Data) & 0xFF;
    }
    {
        if ((pdev->header_type & PCI_HEADER_TYPE_MASK) == PCI_HEADER_TYPE_NORMAL)
        {
            uint32_t bar0 = pci_config_read32(pdev, PCI_HEADER_BASE_ADDRESS_0);
            pci_int_decode_bar(pdev, bar0, PCI_HEADER_BASE_ADDRESS_0, &pdev->bars[0]);
            
            uint32_t bar1 = pci_config_read32(pdev, PCI_HEADER_BASE_ADDRESS_1);
            pci_int_decode_bar(pdev, bar1, PCI_HEADER_BASE_ADDRESS_1, &pdev->bars[1]);
            
            uint32_t bar2 = pci_config_read32(pdev, PCI_HEADER_BASE_ADDRESS_2);
            pci_int_decode_bar(pdev, bar2, PCI_HEADER_BASE_ADDRESS_2, &pdev->bars[2]);
            
            uint32_t bar3 = pci_config_read32(pdev, PCI_HEADER_BASE_ADDRESS_3);
            pci_int_decode_bar(pdev, bar3, PCI_HEADER_BASE_ADDRESS_3, &pdev->bars[3]);
            
            uint32_t bar4 = pci_config_read32(pdev, PCI_HEADER_BASE_ADDRESS_4);
            pci_int_decode_bar(pdev, bar4, PCI_HEADER_BASE_ADDRESS_4, &pdev->bars[4]);
            
            uint32_t bar5 = pci_config_read32(pdev, PCI_HEADER_BASE_ADDRESS_5);
            pci_int_decode_bar(pdev, bar5, PCI_HEADER_BASE_ADDRESS_5, &pdev->bars[5]);

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

    return true;
}

extern bus_type_t pci_bus_type;

void pci_check_buses()
{
    for (uint32_t bus = 0; bus < 256; bus++)
    {
        for (uint32_t slot = 0; slot < 32; slot++)
        {
            pci_device_t pdev;
            uint16_t function = 0;
            bool result = pci_check_bus(bus, slot, function, &pdev);
            if (result == false)
            {
                continue;
            }

            // trace_debug(MODULE, "pci_devices[%u] = %p", pci_device_count, &pdev);
            device_t *dev = &pdev.dev;
            memset(dev, 0, sizeof(*dev));
            pci_device_id_t *id = &pdev.id;
            id->vendor = pdev.vendor_id;
            id->device = pdev.device_id;
            id->class = (pdev.class_code << 16) | (pdev.sub_class << 8) | pdev.prog_if;
            dev->bus = &pci_bus_type;
            device_register(dev);
            pci_device_count++;

            pci_devices[pci_device_count] = kmalloc(sizeof(pci_device_t));
            memcpy(pci_devices[pci_device_count], &pdev, sizeof(pci_device_t));

            // trace_debug(MODULE, "vendor: 0x%x device: 0x%x", pdev.vendor_id, pdev.device_id);
            trace_debug(MODULE, "bus: 0x%x, slot: 0x%x, function: 0x%x", pdev.bdf.bus, pdev.bdf.device, pdev.bdf.function);
            trace_debug(MODULE, "device_id: 0x%x, vendor: 0x%x", pdev.device_id, pdev.vendor_id);
            trace_debug(MODULE, "status: 0x%x, command: 0x%x", pdev.status, pdev.command);
            trace_debug(MODULE, "class_code: 0x%x, sub_class: 0x%x, prog_if: 0x%x, revision: 0x%x", pdev.class_code, pdev.sub_class, pdev.prog_if, pdev.revision);
            trace_debug(MODULE, "bist: 0x%x, header_type: 0x%x, latency_timer: 0x%x, cache_line_size: 0x%x", pdev.bist, pdev.header_type, pdev.latency_timer, pdev.cache_line_size);
            log_debug(NO_MODULE, "");
            if (pdev.header_type & 0x80)
            {
                function = 1;
                for (; function < 8; function++)
                {
                    pci_device_t pdev_func;
                    result = pci_check_bus(bus, slot, function, &pdev_func);
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
