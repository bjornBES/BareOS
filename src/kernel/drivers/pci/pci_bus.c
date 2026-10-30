/*
 * File: pci_bus.c
 * File Created: 26 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/pci/pci.h"

#include "drivers/base/bus.h"

#include "kerrno.h"

#include "init.h"

#include <binary.h>

#define MODULE "pci-bus"

typedef struct pci_dynid
{
    list_node_t node;
    pci_device_id_t id;
} pci_dynid_t;

INTERNAL_INLINE pci_device_id_t *pci_match_one_id(pci_device_id_t *pci_id, pci_device_id_t *pci_dev_id)
{
    if ((pci_id->vendor == PCI_ANY_ID || pci_id->vendor == pci_dev_id->vendor) &&
    (pci_id->device == PCI_ANY_ID || pci_id->device == pci_dev_id->device) &&
    !((pci_id->class ^ pci_dev_id->class_mask) & pci_id->class_mask))
    {
        // ENTER_FUNC("%p { %04x:%04x, %06x, %06x }, %p { %04x:%04x, %06x, %06x }", id, id->vendor, id->device, id->class, id->class_mask, dev_id, dev_id->vendor, dev_id->device, dev_id->class, dev_id->class_mask);
        return pci_id;
    }
    return NULL;
}

pci_device_id_t *pci_do_match_id(pci_device_id_t *pci_ids, pci_device_id_t *pci_dev_id)
{
    ENTER_FUNC("%p, %p", pci_ids, pci_dev_id);
    int index = 0;
    while (pci_ids != NULL)
    {
        if (pci_ids->vendor == 0 && pci_ids->device == 0 && pci_ids->class == 0 && pci_ids->class_mask == 0)
        {
            break;
        }

        if (pci_match_one_id(pci_ids, pci_dev_id) != NULL)
        {
            trace_debug(MODULE, "pci_ids[%u] = 0x%04x:%04x, %06x", index, pci_ids->vendor, pci_ids->device, pci_ids->class);
            trace_debug(MODULE, "pci_dev_id = 0x%04x:%04x, %06x", pci_dev_id->vendor, pci_dev_id->device, pci_dev_id->class);
            return pci_ids;
        }

        pci_ids++;
        index++;
    }

    return NULL;
}

INTERNAL pci_device_id_t *pci_match_device(pci_driver_t *pci_drv, pci_device_t *pci_dev, pci_device_id_t *out)
{
    pci_device_id_t *found_pci_id = NULL;
    pci_device_id_t pci_dev_id = pci_dev->id;

    found_pci_id = pci_do_match_id(pci_drv->id_table, &pci_dev_id);
    if (found_pci_id)
    {
        return found_pci_id;
    }

    return NULL;
}

status_t pci_bus_match(device_t *dev, driver_t *drv)
{
    ENTER_FUNC("%p, %p", dev, drv);
    pci_device_t *pci_dev = to_pci_dev(dev);
    pci_driver_t *pci_drv = to_pci_driver(drv);
    pci_device_id_t *found_pci_id = pci_do_match_id(pci_drv->id_table, &pci_dev->id);

    if (found_pci_id == NULL)
    {
        return KERRNO_UNSUCCESS;
    }
    return KERRNO_SUCCESSES;
}

status_t pci_call_probe(pci_driver_t *pci_drv, pci_device_t *pci_dev, pci_device_id_t *pci_dev_id)
{
    pci_dev->driver = pci_drv;
    status_t ret = pci_drv->probe(pci_dev, pci_dev_id);
    if (ret != KERRNO_SUCCESSES)
    {
        pci_dev->driver = NULL;
        return ret;
    }
    return ret;
}

INTERNAL status_t __pci_device_probe(pci_driver_t *pci_drv, pci_device_t *pci_dev)
{
    pci_device_id_t pci_id_copy;
    if (pci_drv->probe)
    {
        pci_device_id_t *pci_id = pci_match_device(pci_drv, pci_dev, &pci_id_copy);
        if (pci_id)
        {
            return pci_call_probe(pci_drv, pci_dev, pci_id);
        }
        KERRNO_RETURN(ENODEV, "devices id is not in driver's %s id table", pci_drv->name);
    }
    KERRNO_RETURN(KERRNO_NOT_ALLOWED, "driver %s doesn't have the probe function defined", pci_drv->name);
}

status_t pci_device_probe(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    pci_device_t *pci_dev = to_pci_dev(dev);
    pci_driver_t *pci_drv = to_pci_driver(dev->driver);

    status_t ret = __pci_device_probe(pci_drv, pci_dev);

    return ret;
}

bus_type_t pci_bus_type = {
    .name = "pci",
    .driver_override = true,
    .match = pci_bus_match,
    // .uevent		= pci_uevent,
    .probe = pci_device_probe,
    // .remove		= pci_device_remove,
    // .shutdown	= pci_device_shutdown,
    // .irq_get_affinity = pci_device_irq_get_affinity,
    // .dev_groups	= pci_dev_groups,
    // .bus_groups	= pci_bus_groups,
    // .drv_groups	= pci_drv_groups,
    // .pm		= PCI_PM_OPS_PTR,
    // .num_vf		= pci_bus_num_vf,
    // .dma_configure	= pci_dma_configure,
    // .dma_cleanup	= pci_dma_cleanup,
};

status_t pci_register_bus()
{
    bus_register(&pci_bus_type);
    return KERRNO_SUCCESSES;
}

SUBSYS_INITCALL(pci_register_bus);
