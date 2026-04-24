/*
 * File: pci.c
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pci.h"
#include "pci_config.h"

#include "debug/debug.h"
#include "drivers/serial/UART/UART.h"
#include "drivers/drive/ahci/ahci.h"
#include "libs/malloc.h"

#include "libs/IO.h"
#include "libs/memory.h"
#include <util/binary.h>

#define MODULE "PCI"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

#define getAddress(bus, device, func, offset) (uint32_t)(((uint32_t)(bus) << 16) | ((uint32_t)(device) << 11) | ((uint32_t)(func) << 8) | ((uint32_t)(offset) & 0xFC) | ((uint32_t)0x80000000))

uint32_t devs_count;
pci_device_id **devs;

void pci_add_device(pci_device_id *dev)
{
    devs[devs_count] = dev;
    devs_count++;
}

uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset)
{
    uint32_t address = getAddress(bus, device, func, offset);

    outd(CONFIG_ADDRESS, address);
    uint32_t data = ind(CONFIG_DATA);
    return data;
}
uint32_t pci_config_read_device_dword(pci_device_id *pdev, uint8_t offset)
{
    uint32_t address = getAddress(pdev->bus, pdev->device_id, pdev->function, offset);

    outd(CONFIG_ADDRESS, address);
    uint32_t data = ind(CONFIG_DATA);
    return data;
}
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset)
{
    uint32_t address = getAddress(bus, device, func, offset);

    outd(CONFIG_ADDRESS, address);
    uint16_t data = (uint16_t)((ind(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return data;
}

void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint16_t data)
{
    uint32_t address = getAddress(bus, device, func, offset);

    outd(CONFIG_ADDRESS, address);
    outd(CONFIG_DATA, data);
}

uint16_t pci_get_vendor_ID(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_config_read_word(bus, device, function, 0);
    return r0;
}

uint16_t pci_get_device_ID(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_config_read_word(bus, device, function, 2);
    return r0;
}

uint16_t pci_get_class_ID(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_config_read_word(bus, device, function, 0xA);
    return (r0 & ~0x00FF) >> 8;
}

uint16_t pci_get_sub_class_ID(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_config_read_word(bus, device, function, 0xA);
    return (r0 & ~0xFF00);
}

void pci_enable_mmio_bus_mastering(uint16_t bus, uint16_t slot, uint16_t function)
{
    uint16_t command = pci_config_read_word(bus, slot, function, PCI_COMMAND);
    BIT_SET(command, 1);
    BIT_SET(command, 2);
    BIT_UNSET(command, 10);
    pci_config_write_word(bus, slot, function, PCI_COMMAND, command);
}

void pci_enable_interrupts(uint16_t bus, uint16_t slot, uint16_t function)
{
    uint16_t command = pci_config_read_word(bus, slot, function, PCI_COMMAND);
    BIT_UNSET(command, 10);
    pci_config_write_word(bus, slot, function, PCI_COMMAND, command);
}
void pci_disable_interrupts(uint16_t bus, uint16_t slot, uint16_t function)
{
    uint16_t command = pci_config_read_word(bus, slot, function, PCI_COMMAND);
    BIT_SET(command, 10);
    pci_config_write_word(bus, slot, function, PCI_COMMAND, command);
}

void pci_get_bar(pci_device_id *pdev)
{
    uint32_t bar = pci_config_read_device_dword(pdev, PCI_HEADER_BAR0);
}

void pci_init_device(pci_device_id *pdev)
{
    log_debug(MODULE, "PCI Device Found: Bus %d, slot %d, Function %d, Vendor: 0x%X, Device: 0x%X, Class: 0x%X, Subclass: 0x%X",
              pdev->bus, pdev->slot, pdev->function, pdev->vendor_id, pdev->device_id, pdev->class_code, pdev->sub_class);

    uint16_t bus = pdev->bus;
    uint16_t slot = pdev->slot;
    uint16_t function = pdev->function;

    if (pdev->class_code == 0x1)
    {
        // mass storage device
        if (pdev->sub_class == 0x6)
        {
            // SATA
            pci_enable_mmio_bus_mastering(bus, slot, function);
            pci_disable_interrupts(bus, slot, function);

            uint32_t mmio_base = pdev->header.header0.bar5;
            log_debug(MODULE, "ahci bar5 = %X", mmio_base);
            ahci_initialize(pdev);
        }
    }
}

void pci_init_devices()
{
    for (size_t i = 0; i < devs_count; i++)
    {
        pci_device_id *pdev = devs[i];
        pci_init_device(pdev);
    }
}

bool pci_check_bus(uint32_t bus, uint32_t slot, uint8_t func, pci_device_id **device_out)
{
    uint16_t vendor = pci_get_vendor_ID(bus, slot, func);
    if (vendor == 0xffff)
    {
        return false;
    }

    *device_out = (pci_device_id *)malloc(sizeof(pci_device_id));
    pci_device_id *pdev = *device_out;
    {
        pdev->bus = bus;
        pdev->slot = slot;
        pdev->function = func;

        pdev->vendor_id = pci_get_vendor_ID(bus, slot, func);
        pdev->device_id = pci_get_device_ID(bus, slot, func);

        pdev->command = pci_config_read_word(bus, slot, func, PCI_HEADER_COMMAND);
        pdev->status = pci_config_read_word(bus, slot, func, PCI_HEADER_STATUS);

        uint32_t reg2Data = pci_config_read_dword(bus, slot, func, PCI_HEADER_REVISION_ID);
        pdev->class_code = (reg2Data >> 24) & 0xFF;
        pdev->sub_class = (reg2Data >> 16) & 0xFF;
        pdev->prog_if = (reg2Data >> 8) & 0xFF;
        pdev->revision_id = (reg2Data) & 0xFF;

        uint32_t reg3Data = pci_config_read_dword(bus, slot, func, PCI_HEADER_CACHE_LINE_SIZE);
        pdev->bist = (reg3Data >> 24) & 0xFF;
        pdev->header_type = (reg3Data >> 16) & 0xFF;
        pdev->latency_timer = (reg3Data >> 8) & 0xFF;
        pdev->cache_line_size = (reg3Data) & 0xFF;
    }
    if (pdev->header_type != 2)
    {
        {
            uint32_t buffer[12];
            for (size_t i = 0; i < 12; i++)
            {
                buffer[i] = pci_config_read_dword(bus, slot, func, i * 4 + 0x10);
            }
            memcpy(pdev->header.bytes, buffer, sizeof(pci_header));
        }
    }
    return true;
}

void pci_check_buses()
{
    for (uint32_t bus = 0; bus < 256; bus++)
    {
        for (uint32_t slot = 0; slot < 32; slot++)
        {
            uint16_t function = 0;
            pci_device_id *pdev = NULL;
            bool result = pci_check_bus(bus, slot, function, &pdev);
            if (result == false)
            {
                continue;
            }

            log_debug(MODULE, "vendor: 0x%x device: 0x%x", pdev->vendor_id, pdev->device_id);
            pci_add_device(pdev);
            if (pdev->header_type & 0x80)
            {
                for (; function < 8; function++)
                {
                }
            }
            else
            {
            }
        }
    }
}

void pci_init(PCI_bios_info bios_info)
{
    devs = (pci_device_id **)malloc(PCI_MAX_DEVICES * sizeof(pci_device_id));
    pci_check_buses();
}