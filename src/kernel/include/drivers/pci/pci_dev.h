/*
 * File: pci_dev.h
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "pci_id.h"
#include "pci_regs.h"

#include "irq/irqt.h"

#include "resource/resourcet.h"

#include "dev/devicet.h"

#include <types.h>

typedef struct pci_bar
{
    paddr_t base;
    size_t size;
    bool is_mmio; /* false = I/O space */
    bool is_64bit;
    bool prefetchable;
} pci_bar_t;

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

enum
{
    PCI_STD_RESOURCES,
    PCI_STD_RESOURCE_END = PCI_STD_RESOURCES + PCI_STD_NUM_BARS - 1,

    PCI_ROM_RESOURCE,

#define PCI_BRIDGE_IO_WINDOW        (PCI_BRIDGE_RESOURCES + 0)
#define PCI_BRIDGE_MEM_WINDOW       (PCI_BRIDGE_RESOURCES + 1)
#define PCI_BRIDGE_PREF_MEM_WINDOW  (PCI_BRIDGE_RESOURCES + 2)

#define PCI_CB_BRIDGE_IO_0_WINDOW   (PCI_BRIDGE_RESOURCES + 0)
#define PCI_CB_BRIDGE_IO_1_WINDOW   (PCI_BRIDGE_RESOURCES + 1)
#define PCI_CB_BRIDGE_MEM_0_WINDOW  (PCI_BRIDGE_RESOURCES + 2)
#define PCI_CB_BRIDGE_MEM_1_WINDOW  (PCI_BRIDGE_RESOURCES + 3)

#define PCI_P2P_BRIDGE_RESOURCE_NUM 3
#define PCI_BRIDGE_RESOURCE_NUM     4

    PCI_BRIDGE_RESOURCES,
    PCI_BRIDGE_RESOURCE_END = PCI_BRIDGE_RESOURCES +
        PCI_BRIDGE_RESOURCE_NUM - 1,

    /* Total resources associated with a PCI device */
    PCI_NUM_RESOURCES,

    /* Preserve this for compatibility */
    DEVICE_COUNT_RESOURCE = PCI_NUM_RESOURCES,
};

typedef struct pci_driver pci_driver_t;

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

    uint8_t irq_pin;
    uint8_t irq_line;

    uint8_t header_type;

    uint8_t config_ptr;

    uint16_t msi_cap; /* MSI capability offset */
    uint64_t msi_addr_mask;

    uint16_t msix_cap; /* MSI-X capability offset */

    interrupt_vector_t irq;

    size_t irq_count;
    irq_range_t irq_range;
    kernel_irq_t kernel_irq;

    // Flags
    uint64_t multi_function : 1;

    uint64_t is_bus_master : 1;
    uint64_t no_msi : 1;
    uint64_t msi_enabled : 1;
    uint64_t msix_enabled : 1;
    uint64_t res : 64 - 5;

    pci_bar_t bars[PCI_STD_NUM_BARS];
    resource_t resource[DEVICE_COUNT_RESOURCE];
    resource_t driver_exclusive_resource;

    size_t conf_size;

    status_t (*handler)(struct pci_device *dev);

    pci_device_id_t id;

    pci_driver_t *driver;

    device_t dev;

    uint32_t saved_config_space[16];

    void *priv;

    paddr_t rom;
    size_t romlen;
} pci_device_t;

#define to_pci_dev(n) container_of(n, pci_device_t, dev)
