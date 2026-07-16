/*
 * File: device.c
 * File Created: 06 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "device.h"
#include "debug/debug.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "errno/errno.h"

#define MODULE "DEVICE"

uint32_t device_id_counters[DEVICE_TYPE_MAX] = {0};
device_t *device_lists[DEVICE_TYPE_MAX] = {0};
int dev_count;

void device_init()
{
    dev_count = 0;
    log_info(MODULE, "Devices are initialized");
}

int device_register(device_t *dev)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -EINVAL;
    }

    dev->device_id = device_id_counters[dev->type]++;

    snprintf(dev->name, DEVICE_NAME_MAX, "%s%u", dev->class_name, dev->device_id);

    dev->next = device_lists[dev->type];
    device_lists[dev->type] = dev;

    log_info(MODULE, "registered %s (type=%d id=%d)", dev->name, dev->type, dev->device_id);
    dev_count++;
    return RETURN_GOOD;
}

void device_unregister(device_t *dev)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return;
    }

    device_t *priv = NULL;
    device_t *curr = device_lists[dev->type];
    while (curr && curr != dev)
    {
        if (curr == dev)
        {
            break;
        }
        priv = curr;
        curr = curr->next;
    }
    if (curr == dev)
    {
        priv->next = curr;
        free(dev);
    }
}

device_t *device_get_by_name(const char *name)
{
    ENTER_FUNC(MODULE, "\"%s\"", name);
    for (size_t i = 0; i < DEVICE_TYPE_MAX; i++)
    {
        device_t *dev = device_lists[i];
        while (dev)
        {
            if (dev == NULL)
            {
                log_info(MODULE, "device: %u, NULL", i);
                continue;
            }
            if (strcmp(dev->name, name) == RETURN_GOOD)
            {
                return dev;
            }
            dev = dev->next;
        }
    }
    return NULL;
}

device_t *device_get_by_id(device_type_t type, dev_t id)
{
    ENTER_FUNC(MODULE, "0x%x, %i", type, id);
    device_t *curr = device_lists[type];
    while (curr)
    {
        if (curr->device_id == id)
        {
            break;
        }
        curr = curr->next;
    }
    return curr;
}

device_t *device_get_first(device_type_t type)
{
    device_t *curr = device_lists[type];
    while (curr->next)
    {
        curr = curr->next;
    }
    return curr;
}

device_t *device_get_next(device_t *dev)
{
    device_t *curr = device_lists[dev->type];
    return curr;
}

size_t device_read(device_t *dev, void *buf, off_t off, size_t count)
{
    if (!dev || !dev->read)
    {
        return 0;
    }
    return dev->read(buf, off, count, dev);
}

size_t device_write(device_t *dev, void *buf, off_t off, size_t count)
{
    if (!dev || !dev->write)
    {
        return 0;
    }
    return dev->write(buf, off, count, dev);
}

int device_ioctl(device_t *dev, uint32_t cmd, void *arg)
{
    if (!dev || !dev->ioctl)
    {
        return 0;
    }
    return dev->ioctl(cmd, arg, dev);
}

void device_debug()
{
    log_info(MODULE, "device count: %u", dev_count);
    char *DEVICE_TYPE_STRING[8] = {
        [DEVICE_UNKNOWN] = "UNKNOWN",
        [DEVICE_BLOCK] = "BLOCK",
        [DEVICE_CHAR] = "CHAR",
        [DEVICE_TTY] = "TTY",
        [DEVICE_NET] = "NET",
        [DEVICE_VIDEO] = "VIDEO",
        [DEVICE_TIMER] = "TIMER",
        [DEVICE_PSEUDO] = "PSEUDO",
    };
    for (size_t i = 0; i < DEVICE_TYPE_MAX; i++)
    {
        device_t *dev = device_lists[i];
        while (dev)
        {
            if (dev == NULL)
            {
                log_info(MODULE, "device: %u, NULL", i);
                continue;
            }
            log_info(MODULE, "device: %u, id:%x, %s(%u), name:%s, read:%p, write:%p", i, dev->device_id, DEVICE_TYPE_STRING[dev->type], dev->type, dev->name, dev->read, dev->write);
            dev = dev->next;
        }
    }
}
