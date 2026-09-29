/*
 * File: pci_bus.c
 * File Created: 26 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pci/pci.h"

#include "drivers/bus.h"

#include "kerrno.h"

#include "init.h"

#include <binary.h>

#define MODULE "pci-bus"

typedef struct pci_dynid
{
    list_node_t node;
    pci_device_id_t id;
} pci_dynid_t;

INTERNAL_INLINE pci_device_id_t *pci_match_one_id(pci_device_id_t *id, pci_device_id_t *dev_id)
{
    if ((id->vendor == PCI_ANY_ID || id->vendor == dev_id->vendor) &&
        (id->device == PCI_ANY_ID || id->device == dev_id->device) &&
        !((id->class ^ dev_id->class_mask) & id->class_mask))
    {
        return id;
    }
    return NULL;
}

pci_device_id_t *pci_do_match_id(pci_device_id_t *ids, pci_device_id_t *dev_id)
{
    int index = 0;
    while (ids != NULL)
    {
        if (ids->vendor == 0 && ids->device == 0 && ids->class == 0 && ids->class_mask == 0)
        {
            break;
        }

        if (pci_match_one_id(ids, dev_id) != NULL)
        {
            trace_debug(MODULE, "ids[%u] = 0x%04x:%04x, %06x", index, ids->vendor, ids->device, ids->class);
            trace_debug(MODULE, "dev_id = 0x%04x:%04x, %06x", dev_id->vendor, dev_id->device, dev_id->class);
            return ids;
        }

        ids++;
        index++;
    }

    return NULL;
}

INTERNAL pci_device_id_t *pci_match_device(pci_driver_t *pci_drv, pci_device_t *pci_dev, pci_device_id_t *out)
{
    pci_device_id_t *found_id = NULL;
    pci_device_id_t dev_id = pci_dev->id;

    found_id = pci_do_match_id(pci_drv->id_table, &dev_id);
    if (found_id)
    {
        return found_id;
    }

    return NULL;
}

status_t pci_bus_match(device_t *dev, driver_t *drv)
{
    ENTER_FUNC("%p, %p", dev, drv);
    pci_device_t *pci_dev = to_pci_dev(dev);
    pci_driver_t *pci_drv = to_pci_driver(drv);
    pci_device_id_t *found_id = pci_do_match_id(pci_drv->id_table, &pci_dev->id);

    if (found_id)
    {
        return KERRNO_SUCCESSES;
    }
    return KERRNO_UNSUCCESS;
}

status_t pci_call_probe(pci_driver_t *pci_drv, pci_device_t *pci_dev, pci_device_id_t *id)
{
    pci_dev->driver = pci_drv;
    status_t ret = pci_drv->probe(pci_dev, id);
    if (ret != KERRNO_SUCCESSES)
    {
        pci_dev->driver = NULL;
        return ret;
    }
    return ret;
}

INTERNAL status_t __pci_device_probe(pci_driver_t *pci_drv, pci_device_t *pci_dev)
{
    pci_device_id_t id_copy;
    if (pci_drv->probe)
    {
        pci_device_id_t *id = pci_match_device(pci_drv, pci_dev, &id_copy);
        if (id)
        {
            return pci_call_probe(pci_drv, pci_dev, id);
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
