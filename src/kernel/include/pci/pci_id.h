/*
 * File: pci_id.h
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

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

typedef struct pci_device_id
{
    uint32_t vendor;
    uint32_t device;
    uint32_t class;
    uint32_t class_mask;
} pci_device_id_t;
