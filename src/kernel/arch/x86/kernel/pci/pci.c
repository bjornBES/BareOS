/*
 * File: pci.c
 * File Created: 23 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/arch_pci.h"

#include "kernel/io.h"

#define CONFIG_ADDRESS                        0xCF8
#define CONFIG_DATA                           0xCFC

#define getAddress(bus, device, func, offset) (uint32_t)(((uint32_t)(bus) << 16) | ((uint32_t)(device) << 11) | ((uint32_t)(func) << 8) | ((uint32_t)(offset) & 0xFC) | ((uint32_t)0x80000000))

inline uint32_t pci_arch_read_config32(pci_device_t *dev, uint16_t offset)
{
    uint32_t address = getAddress(dev->bdf.bus, dev->bdf.device, dev->bdf.function, offset);

    outd(CONFIG_ADDRESS, address);
    uint32_t data = ind(CONFIG_DATA);
    return data;
}

inline int pci_arch_write_config32(pci_device_t *dev, uint16_t offset, uint32_t val)
{
    uint32_t address = getAddress(dev->bdf.bus, dev->bdf.device, dev->bdf.function, offset);

    outd(CONFIG_ADDRESS, address);
    outd(CONFIG_DATA, val);
    return 0;
}
