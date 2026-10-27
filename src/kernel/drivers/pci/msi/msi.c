/*
 * File: msi.c
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/pci/msi/msi.h"
#include "drivers/pci/pci.h"

#include "cpu/cpu.h"
#include "irq/irq.h"

#include "kerrno.h"

#define MODULE "pci-msi"

status_t msi_pci_read8(pci_device_t *dev, uint16_t offset, uint8_t *out)
{
    return pci_config_read8(dev, dev->msi_cap + offset, out);
}

status_t msi_pci_read16(pci_device_t *dev, uint16_t offset, uint16_t *out)
{
    return pci_config_read16(dev, dev->msi_cap + offset, out);
}

status_t msi_pci_read32(pci_device_t *dev, uint16_t offset, uint32_t *out)
{
    return pci_config_read32(dev, dev->msi_cap + offset, out);
}

status_t msi_pci_write8(pci_device_t *dev, uint16_t offset, uint8_t val)
{
    return pci_config_write8(dev, dev->msi_cap + offset, val);
}

status_t msi_pci_write16(pci_device_t *dev, uint16_t offset, uint16_t val)
{
    return pci_config_write16(dev, dev->msi_cap + offset, val);
}

status_t msi_pci_write32(pci_device_t *dev, uint16_t offset, uint32_t val)
{
    return pci_config_write32(dev, dev->msi_cap + offset, val);
}

status_t msi_handler(intr_frame_t *frame, void *ctx)
{
    ENTER_FUNC("%p, %p", frame, ctx);
    pci_device_t *dev = ctx;
    status_t ret = dev->handler(dev);
    irq_eoi(frame->interrupt);
    return ret;
}

status_t pci_msi_register_irq(pci_device_t *dev, interrupt_vector_t *out_line, kernel_irq_t *out_pin)
{
    *out_pin = dev->kernel_irq;
    irq_register_handler(dev->kernel_irq, IRQ_SOURCE_MSI, msi_handler, dev, IRQ_TRIGGER_EDGE, IRQ_POLARITY_HIGH, ANY_CPU);
    return KERRNO_SUCCESSES;
}

status_t pci_msi_set_enable(pci_device_t *dev, uint8_t enable)
{
    uint32_t control;

    status_t ret = msi_pci_read32(dev, 0, &control);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    control &= ~PCI_MSI_FLAGS_ENABLE;
    if (enable)
    {
        control |= PCI_MSI_FLAGS_ENABLE;
    }
    ret = msi_pci_write32(dev, 0, control);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    dev->msi_enabled = enable;

    return KERRNO_SUCCESSES;
}

status_t msi_enable(pci_device_t *dev)
{
    return pci_msi_set_enable(dev, 1);
}

status_t msi_disable(pci_device_t *dev)
{
    return pci_msi_set_enable(dev, 0);
}

int pci_msi_vec_count(pci_device_t *dev)
{
    int ret;
    uint16_t msgctl;

    if (!dev->msi_cap)
    {
        return -EINVAL;
    }

    msi_pci_read16(dev, PCI_MSI_FLAGS, &msgctl);
    trace_debug(MODULE, "msgctl = 0x%x", msgctl);
    ret = 1 << FLAG_GET(msgctl, PCI_MSI_FLAGS_QMASK);

    return ret;
}

status_t pci_enable_msi_range(pci_device_t *dev, size_t min_vecs, size_t max_vecs)
{
    ENTER_FUNC("%p, %u, %u", dev, min_vecs, max_vecs);
    if (dev->msix_enabled)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "Can't enable MSI, while MIS-X is already enabled");
    }

    int count = pci_msi_vec_count(dev);
    trace_debug(MODULE, "count = %u", count);

    irq_range_t range = {.first = min_vecs, .last = max_vecs};
    irq_pick_free_range_size(range, count, IRQ_SOURCE_MSI, ANY_CPU, &dev->kernel_irq);

    dev->irq_range.first = dev->kernel_irq;
    dev->irq_range.last = dev->kernel_irq + count;
    dev->irq_count = count;

    pci_msi_set_enable(dev, 1);
    return KERRNO_SUCCESSES;
}
