/*
 * File: pcidev_msi.c
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/pci/msi/pcidev_msi.h"
#include "drivers/pci/pci.h"
#include "kerrno.h"

#define MODULE "pci-msi"

status_t pci_msi_init(pci_device_t *pci_dev)
{
    ENTER_FUNC("%p", pci_dev);
    status_t ret = pci_find_capability(pci_dev, PCI_CAP_ID_MSI, &pci_dev->msi_cap);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    uint16_t flags;
    ret = pci_config_read16(pci_dev, pci_dev->msi_cap + PCI_MSI_FLAGS, &flags);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    if (FLAG_IS_SET(flags, PCI_MSI_FLAGS_ENABLE))
    {
        FLAG_UNSET(flags, PCI_MSI_FLAGS_ENABLE);
        pci_config_write16(pci_dev, pci_dev->msi_cap + PCI_MSI_FLAGS, flags);
    }

    if (FLAG_GET(flags, PCI_MSI_FLAGS_64BIT) == 0)
    {
        pci_dev->msi_addr_mask = BIT_RANGE(31, 0);
    }
    return KERRNO_SUCCESSES;
}

status_t pci_msix_init(pci_device_t *pci_dev)
{
    ENTER_FUNC("%p", pci_dev);
    status_t ret = pci_find_capability(pci_dev, PCI_CAP_ID_MSIX, &pci_dev->msix_cap);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    uint16_t flags;
    ret = pci_config_read16(pci_dev, pci_dev->msix_cap + PCI_MSIX_FLAGS, &flags);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    if (FLAG_IS_SET(flags, PCI_MSIX_FLAGS_ENABLE))
    {
        FLAG_UNSET(flags, PCI_MSIX_FLAGS_ENABLE);
        pci_config_write16(pci_dev, pci_dev->msix_cap + PCI_MSIX_FLAGS, flags);
    }

    return KERRNO_SUCCESSES;
}
