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

#include "pci_id.h"
#include "pci_dev.h"
#include "pci_regs.h"
#include "pci_ids.h"

#define PCI_IRQ_INTX     (1 << 0) /* Allow INTx interrupts */
#define PCI_IRQ_MSI      (1 << 1) /* Allow MSI interrupts */
#define PCI_IRQ_MSIX     (1 << 2) /* Allow MSI-X interrupts */
#define PCI_IRQ_AFFINITY (1 << 3) /* Auto-assign affinity */

typedef struct pci_dynids
{
    spinlock_t lock; /* Protects list, index */
    list_t list;     /* For IDs added at runtime */
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

typedef status_t (*pci_func_t)(pci_device_t *, void *ctx);

status_t pci_initialize();

status_t pci_find_device(uint16_t vendor_id, uint16_t device_id, uint32_t index, pci_device_t **out);

status_t pci_find_class(uint8_t class_code, uint8_t subclass, uint8_t prog_if, uint32_t index, pci_device_t **out);

status_t pci_get_device(uint8_t bus, uint8_t dev, uint8_t func, pci_device_t **out);

status_t pci_for_each(pci_func_t callback, void *ctx);

status_t pci_config_read8(pci_device_t *dev, uint16_t offset, uint8_t *out);
status_t pci_config_read16(pci_device_t *dev, uint16_t offset, uint16_t *out);
status_t pci_config_read32(pci_device_t *dev, uint16_t offset, uint32_t *out);

status_t pci_config_write8(pci_device_t *dev, uint16_t offset, uint8_t val);
status_t pci_config_write16(pci_device_t *dev, uint16_t offset, uint16_t val);
status_t pci_config_write32(pci_device_t *dev, uint16_t offset, uint32_t val);

status_t pci_get_bar(pci_device_t *dev, uint8_t bar_index, pci_bar_t *out);

status_t pci_map_bar(pci_device_t *dev, uint8_t bar_index, vaddr_t *out_vaddr);

status_t pci_find_capability(pci_device_t *dev, uint8_t cap_id, uint16_t *out_offset);

status_t pci_find_ext_capability(pci_device_t *dev, uint16_t cap_id, uint16_t *out_offset);

status_t pci_enable_device(pci_device_t *dev);
status_t pci_set_bus_master(pci_device_t *dev, uint8_t enable);

status_t pci_get_irq(pci_device_t *dev, interrupt_vector_t *out_line, kernel_irq_t *out_pin);

status_t pci_enable_interrupt(pci_device_t *dev);
status_t pci_enable_dma(pci_device_t *dev);
status_t pci_set_command_flags(pci_device_t *dev, pci_command_bits_t flags);
status_t pci_clear_command_flags(pci_device_t *dev, pci_command_bits_t flags);

// pci/msi/msi.c

status_t pci_msi_register_irq(pci_device_t *dev, interrupt_vector_t *out_line, kernel_irq_t *out_pin);

status_t pci_init_irq(pci_device_t *dev, size_t min_vecs, size_t max_vecs, uint32_t flags);

// arch/**/pci/msi.c

// status_t msi_arch_register(pci_device_t *dev, irq_trigger_t trigger, irq_polarity_t polarity, interrupt_vector_t vector, cpu_logical_id_t target);

// pci/pci_bus.c

status_t pci_register_driver(pci_driver_t *driver);
status_t pci_unregister_driver();

#define MODULE_PCI_DRIVER(__pci_driver)                                     \
    MODULE_DRIVER(__pci_driver, pci_register_driver, pci_unregister_driver)
