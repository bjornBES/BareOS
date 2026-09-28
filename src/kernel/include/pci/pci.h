/*
 * File: pci.h
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "drivers/driver.h"
#include "dev/device.h"

#include "pci_regs.h"
#include "pci_ids.h"

#define PCI_HEADER_VENDOR_ID       0x00 // 2

#define PCI_HEADER_DEVICE_ID       0x02 // 2

#define PCI_HEADER_COMMAND         0x04 // 2

#define PCI_HEADER_STATUS          0x06 // 2

#define PCI_HEADER_REVISION_ID     0x08 // 1
#define PCI_HEADER_PROG_IF         0x09 // 1

#define PCI_HEADER_SUBCLASS        0x0A // 1
#define PCI_HEADER_CLASS           0x0B // 1

#define PCI_HEADER_CACHE_LINE_SIZE 0x0C // 1
#define PCI_HEADER_LATENCY_TIMER   0x0D // 1

#define PCI_HEADER_HEADER_TYPE     0x0E // 1
#define PCI_HEADER_BIST            0x0F // 1

#define PCI_DEVICE(vend, dev)         \
    .vendor = (vend), .device = (dev)

#define PCI_DEVICE_SUB(vend, dev, subvend, subdev) \
    .vendor = (vend), .device = (dev)

#define PCI_DEVICE_CLASS(dev_class, dev_class_mask)       \
    .class = (dev_class), .class_mask = (dev_class_mask), \
    .vendor = PCI_ANY_ID, .device = PCI_ANY_ID

#define PCI_VDEVICE(vend, dev)                            \
    .vendor = PCI_VENDOR_ID_##vend, .device = (dev), 0, 0

#define PCI_ANY_ID   (~0)

#define PCI_MAX_NAME 32
#define PCI_ID_IDENT "PCI:"

typedef union pci_bdf
{
    struct
    {
        uint8_t bus;
        uint8_t device;
        uint8_t function;
    };
    uint32_t bdf;
} pci_bdf_t;


typedef struct pci_device_id
{
    uint32_t vendor;
    uint32_t device;
    uint32_t class;
    uint32_t class_mask;
} pci_device_id_t;

typedef struct pci_dynids
{
    spinlock_t lock;  /* Protects list, index */
    list_t list; /* For IDs added at runtime */
} pci_dynids_t;

typedef struct pci_device pci_device_t;

typedef struct pci_driver
{
    char *name;
    pci_device_id_t *id_table;

    status_t (*probe)(pci_device_t *dev, pci_device_id_t *id);
    void (*remove)(pci_device_t *dev);

    driver_t driver;
    pci_dynids_t dynids;
} pci_driver_t;

#define to_pci_driver(__drv)                                   \
    (__drv ? container_of(__drv, pci_driver_t, driver) : NULL)

typedef struct pci_bar
{
    paddr_t base;
    size_t size;
    bool is_mmio; /* false = I/O space */
    bool is_64bit;
    bool prefetchable;
} pci_bar_t;

typedef struct pci_device
{
    pci_bdf_t bdf;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t sub_class;
    uint8_t class_code;

    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t bist;
    
    uint8_t header_type;

    pci_bar_t bars[PCI_STD_NUM_BARS];

    size_t conf_size;

    pci_device_id_t id;

    pci_driver_t *driver;

    device_t dev;

    uint32_t saved_config_space[16];
} pci_device_t;

#define to_pci_dev(n) container_of(n, pci_device_t, dev)

typedef status_t (*pci_func_t)(pci_device_t *, void *ctx);

status_t pci_initialize();

status_t pci_find_device(uint16_t vendor_id, uint16_t device_id, uint32_t index, pci_device_t **out);

status_t pci_find_class(uint8_t class_code, uint8_t subclass, uint8_t prog_if, uint32_t index, pci_device_t **out);

status_t pci_get_device(uint8_t bus, uint8_t dev, uint8_t func, pci_device_t **out);

status_t pci_for_each(pci_func_t callback, void *ctx);

uint8_t pci_config_read8(pci_device_t *dev, uint16_t offset);
uint16_t pci_config_read16(pci_device_t *dev, uint16_t offset);
uint32_t pci_config_read32(pci_device_t *dev, uint16_t offset);

status_t pci_config_write8(pci_device_t *dev, uint16_t offset, uint8_t val);
status_t pci_config_write16(pci_device_t *dev, uint16_t offset, uint16_t val);
status_t pci_config_write32(pci_device_t *dev, uint16_t offset, uint32_t val);

status_t pci_get_bar(pci_device_t *dev, uint8_t bar_index, pci_bar_t *out);

status_t pci_map_bar(pci_device_t *dev, uint8_t bar_index, void *out_vaddr);

status_t pci_find_capability(pci_device_t *dev, uint8_t cap_id, uint16_t *out_offset);

status_t pci_find_ext_capability(pci_device_t *dev, uint16_t cap_id, uint16_t *out_offset);

status_t pci_enable_device(pci_device_t *dev);
status_t pci_set_bus_master(pci_device_t *dev, uint8_t enable);

status_t pci_get_irq(pci_device_t *dev, interrupt_vector_t *out_line, uint8_t *out_pin);

status_t pci_enable_interrupt(pci_device_t *dev);
status_t pci_enable_dma(pci_device_t *dev);
status_t pci_set_command_flags(pci_device_t *dev, pci_command_bits_t flags);
status_t pci_clear_command_flags(pci_device_t *dev, pci_command_bits_t flags);

status_t pci_register_driver(pci_driver_t *driver);
status_t pci_unregister_driver();

#define MODULE_PCI_DRIVER(__pci_driver)                                     \
    MODULE_DRIVER(__pci_driver, pci_register_driver, pci_unregister_driver)
