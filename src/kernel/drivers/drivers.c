/*
 * File: drivers.c
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/driver.h"

#include "lists/list.h"

#include "init.h"

#include "kerrno.h"

#include "string.h"

#define MODULE       "driver"

#define to_driver(n) container_of(n, driver_t, node)

list_t driver_registry;

__init status_t drivers_initialize()
{
    list_initialize(&driver_registry);
    return KERRNO_SUCCESSES;
}

EARLY_INITCALL(drivers_initialize);

status_t drivers_register(driver_t *dri)
{
    ENTER_FUNC("%p", dri);
    status_t ret = list_push_tail(&driver_registry, &dri->node);
    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }
    return KERRNO_SUCCESSES;
}

driver_t *driver_get(bus_type_t *bus)
{
    ENTER_FUNC("%p", bus);
    spinlock_acquire(&driver_registry.list_lock);
    list_node_t *node = driver_registry.head;
    while (node != NULL)
    {
        driver_t *dri = (driver_t *)container_of(node, driver_t, node);
        trace_debug(MODULE, "dri = %p", dri);
        trace_debug(MODULE, "dri->bus = %p", dri->bus);
        if (dri->bus == bus)
        {
            spinlock_release(&driver_registry.list_lock);
            return dri;
        }
        node = node->next;
    }
    spinlock_release(&driver_registry.list_lock);
    return NULL;
}

status_t driver_next(driver_t **drv, bus_type_t *bus)
{
    if (*drv == NULL)
    {
        *drv = driver_get(bus);
        return KERRNO_SUCCESSES;
    }

    spinlock_acquire(&driver_registry.list_lock);

    if (!(*drv)->node.next)
    {
        spinlock_release(&driver_registry.list_lock);
        KERRNO_RETURN(KERRNO_PERMISSION_DENIED, "next driver is null");
    }

    do
    {
        *drv = to_driver((*drv)->node.next);
    } while (!(*drv) && (*drv)->bus != bus);

    if (!(*drv))
    {
        spinlock_release(&driver_registry.list_lock);
        KERRNO_RETURN(KERRNO_PERMISSION_DENIED, "next driver is null");
    }

    spinlock_release(&driver_registry.list_lock);
    return KERRNO_SUCCESSES;
}
