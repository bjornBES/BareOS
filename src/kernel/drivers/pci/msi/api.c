/*
 * File: api.c
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/pci/pci.h"
#include "drivers/pci/msi/msi.h"

#include "kerrno.h"

#define MODULE "pci-api"

status_t pci_init_irq(pci_device_t *dev, size_t min_vecs, size_t max_vecs, uint32_t flags)
{
    ENTER_FUNC("%p, 0x%x", dev, flags);
    status_t ret = 0;

/*     if (flags & PCI_IRQ_MSIX)
    {
        nvecs = __pci_enable_msix_range(dev, NULL, min_vecs, max_vecs, flags);
        if (nvecs > 0)
        {
            return nvecs;
        }
    } */

    if ((flags & PCI_IRQ_MSI) == PCI_IRQ_MSI)
    {
        ret = pci_enable_msi_range(dev, min_vecs, max_vecs);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
        return KERRNO_SUCCESSES;
    }

    /* use INTx IRQ if allowed */
/*     if (flags & PCI_IRQ_INTX)
    {
        if (min_vecs == 1 && dev->irq)
        {
            pci_intx(dev, 1);
            return 1;
        }
    } */

    KERRNO_RETURN(KERRNO_DEVICE_FULL, "");
}