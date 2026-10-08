/*
 * File: msi.c
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/msi_arch.h"

#include "kernel/smp/ipi.h"

#include "cpu/cpu.h"

#include "pci/pci_regs.h"
#include "pci/msi/msi.h"

#include "kerrno.h"

status_t msi_arch_set_vector(pci_device_t *dev, interrupt_vector_t vector, cpu_logical_id_t target, irq_trigger_t trigger, irq_polarity_t polarity)
{
    cpu_entry_t *entry = cpu_get_entry(target);
    uint32_t message_data = (trigger << 15) | (polarity << 14) | (DELI_FIXED << 8) |  vector;
    if (dev->msi_addr_mask == BIT_RANGE(31, 0))
    {
        msi_pci_write32(dev, PCI_MSI_DATA_32, message_data);
    }
    else
    {
        msi_pci_write32(dev, PCI_MSI_DATA_64, message_data);
    }
    return msi_pci_write32(dev, PCI_MSI_ADDRESS_LO, (0xFEE << 20) | (entry->arch_id << 12));
}

/* status_t msi_arch_register(pci_device_t *dev, irq_trigger_t trigger, irq_polarity_t polarity, interrupt_vector_t vector, cpu_logical_id_t target)
{

} */