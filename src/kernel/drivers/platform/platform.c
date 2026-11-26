/*
 * File: platform.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/platform/platform.h"

#include "drivers/base/class.h"
#include "drivers/base/driver.h"
#include "drivers/base/bus.h"

#include "resource/resource.h"

#include "dev/device.h"

#include "memory.h"
#include "string.h"

#include "kerrno.h"

#define MODULE "platform"

extern bus_type_t platform_bus_type;

typedef struct platform_object
{
    platform_device_t pdev;
    char name[];
} platform_object_t;

// device_t platform_bus = {
//     .init_name = "platform",
// };

status_t platform_device_alloc(const char *name, int id, platform_device_t **out_platform_dev)
{
    platform_object_t *pobj = kzmalloc(sizeof(platform_object_t) + strlen(name) + 1);
    if (!pobj)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED, "");
    }
    platform_device_t *pdev = &pobj->pdev;
    strcpy(pobj->name, name);
    pdev->name = pobj->name;
    pdev->id = id;
    status_t ret = device_initialize(&pdev->dev);
    if (ret)
    {
        LOG_RETURN(ret);
    }
    // pdev->dev.ops.release = platform_device_release;
    // setup_pdev_dma_masks(&pa->pdev);

    *out_platform_dev = pdev;
    return KERRNO_SUCCESS;
}

status_t platform_device_add(platform_device_t *pdev)
{
    device_t *dev = &pdev->dev;
    uint32_t i;
    status_t ret;

    if (!dev->parent)
    {
        // dev->parent = &platform_bus;
    }

    dev->bus = &platform_bus_type;

    switch (pdev->id)
    {
        default :
            device_set_name(dev, "%s.%d", pdev->name, pdev->id);
            break;
        case PLATFORM_DEVID_NONE :
            device_set_name(dev, "%s", pdev->name);
            break;
        case PLATFORM_DEVID_AUTO :
            // ret = ida_alloc(&platform_devid_ida, GFP_KERNEL);
            // if (ret < 0)
            // {
            //     return ret;
            // }
            // pdev->id = ret;
            // pdev->id_auto = true;
            // device_set_name(dev, "%s.%d.auto", pdev->name, pdev->id);
            FUNC_NOT_IMPLEMENTED();
            break;
    }

    for (i = 0; i < pdev->num_resources; i++)
    {
        resource_t *p;
        resource_t *r = &pdev->resource[i];

        if (r->name == NULL)
        {
            r->name = (char *)dev_name(dev);
        }

        p = r->parent;
        if (!p)
        {
            if (r->space & RES_SPACE_RAM)
            {
                p = &mem_space.root;
            }
            else if (r->space == RES_SPACE_IO)
            {
                p = &io_space.root;
            }
        }

        if (p)
        {
            ret = resource_insert(p, r);
            if (ret)
            {
                pr_err("failed to claim resource %d: %pR", i, r);
                goto failed;
            }
        }
    }

    pr_debug("Registering platform device '%s'. Parent at %s", dev_name(dev), dev_name(dev->parent));

    ret = device_add(dev);
    if (ret)
    {
        goto failed;
    }

    return KERRNO_SUCCESS;

failed:
    while (i--)
    {
        resource_t *r = &pdev->resource[i];
        if (r->parent)
        {
            resource_release(r);
        }
    }

    LOG_RETURN(ret);
}

status_t platform_device_register(platform_device_t *platform_dev)
{
    platform_dev->dev.bus = &platform_bus_type;
    dev_set_ready_to_probe(&platform_dev->dev);
    return device_register(&platform_dev->dev);
}

status_t platform_register_driver(platform_driver_t *driver)
{
    driver->driver.bus = &platform_bus_type;
    return driver_register(&driver->driver);
}

status_t platform_unregister_driver(platform_driver_t *driver)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

status_t __init platform_bus_init()
{
    status_t ret;

    // dev_set_no_driver(&platform_bus);
    // platform_bus.class = &virtual_class;

    // ret = device_register(&platform_bus);
    ret = bus_register(&platform_bus_type);
    KERRNO_CHECK_AND_RETURN(ret);

    return ret;
}

SUBSYS_INITCALL(platform_bus_init);
