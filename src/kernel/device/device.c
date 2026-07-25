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

#include "VFS/inode_flags.h"
#include "fs/devfs/devfs.h"

#define MODULE             "DEVICE"

#define MAX_DEVICE_CLASSES 32

typedef struct
{
    char name[DEVICE_NAME_MAX]; // "hpet", "pit", "lapic", "tty", ...
    uint32_t next_id;
} device_class_counter_t;

static device_class_counter_t class_counters[MAX_DEVICE_CLASSES];
static int class_counter_count = 0;

uint32_t device_id_counters[DEVICE_TYPE_MAX] = {0};
device_t *device_lists[DEVICE_TYPE_MAX] = {0};
int dev_count;

void device_init()
{
    dev_count = 0;
    log_info(MODULE, "Devices are initialized");
}

device_t *device_create(device_type_t type, uint32_t flags)
{
    device_t *device = malloc(sizeof(device_t));
    device->type = type;
    device->flags = flags;
    return device;
}

uint32_t get_next_class_id(const char *class_name)
{
    for (int i = 0; i < class_counter_count; i++)
    {
        if (strcmp(class_counters[i].name, class_name) == 0)
        {
            return class_counters[i].next_id++;
        }
    }

    // new class name, start at 0
    strncpy(class_counters[class_counter_count].name, class_name, DEVICE_NAME_MAX - 1);
    class_counters[class_counter_count].next_id = 1; // returning 0 this call
    class_counter_count++;
    return 0;
}

int device_register_under_dev_id(device_t *dev, dev_t device_id)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -EINVAL;
    }

    log_info(MODULE, "device is a partition of %s", dev->class_name);
    uint32_t class_id = get_next_class_id(dev->class_name);
    snprintf(dev->name, DEVICE_NAME_MAX, "%sp%u", dev->class_name, class_id);
    uint64_t raw_device = DEVICE(device_id);
    dev->device_id = MKDEV(dev->type, raw_device, class_id);

    dev->next = device_lists[dev->type];
    device_lists[dev->type] = dev;

    uint64_t major = MAJOR(dev->device_id);
    uint64_t minor = MINOR(dev->device_id);
    log_info(MODULE, "registered %s (type=%d id=%d dev_id=%u:%u:%u)", dev->name, dev->type, dev->id, major, raw_device, minor);
    dev_count++;

    switch (dev->type)
    {
        case DEVICE_TTY :
        case DEVICE_CHAR : // DEVICE_SERIAL?
            devfs_create_entry(dev, DT_CHR);
            break;
        case DEVICE_BLOCK :
            devfs_create_entry(dev, DT_CHR);
            break;
        default :
            break; // not everything needs a /dev/ entry
    }

    return RETURN_GOOD;
}

int device_register(device_t *dev)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -EINVAL;
    }

    if (dev->class_name != NULL)
    {
        uint32_t class_id = get_next_class_id(dev->class_name);
        snprintf(dev->name, DEVICE_NAME_MAX, "%s%u", dev->class_name, class_id);
    }
    else
    {
        log_err(MODULE, "class name is NULL for type %u", dev->type);
    }
    dev->id = device_id_counters[dev->type];
    device_id_counters[dev->type]++;
    dev->device_id = MKDEV(dev->type, dev->id, 0);

    dev->next = device_lists[dev->type];
    device_lists[dev->type] = dev;

    uint64_t major = MAJOR(dev->device_id);
    uint64_t device = DEVICE(dev->device_id);
    uint64_t minor = MINOR(dev->device_id);
    log_info(MODULE, "registered %s (type=%d id=%d dev_id=%u:%u:%u)", dev->name, dev->type, dev->id, major, device, minor);
    dev_count++;

    switch (dev->type)
    {
        case DEVICE_TTY :
        case DEVICE_CHAR : // DEVICE_SERIAL?
            devfs_create_entry(dev, DT_CHR);
            break;
        case DEVICE_BLOCK :
            get_next_class_id(dev->name);
            devfs_create_entry(dev, DT_CHR);
            break;
        default :
            break; // not everything needs a /dev/ entry
    }

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
        if (curr->id == id)
        {
            break;
        }
        curr = curr->next;
    }
    return curr;
}

device_t *device_get_by_dev_id(dev_t id)
{
    uint64_t major = MAJOR(id);
    uint64_t device = DEVICE(id);
    uint64_t minor = MINOR(id);
    ENTER_FUNC(MODULE, "0x%lx (%u:%u:%u)", id, major, device, minor);
    device_t *curr = device_lists[major];
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

ssize_t device_read(device_t *dev, void *buf, off_t off, size_t count)
{
    if (!dev || !dev->read)
    {
        return 0;
    }
    return dev->read(buf, off, count, dev);
}

ssize_t device_write(device_t *dev, void *buf, off_t off, size_t count)
{
    if (!dev)
    {
        log_debug(MODULE, "dev is NULL");
        return 0;
    }
    if (!dev->write)
    {
        log_debug(MODULE, "dev (%s) doesn't have write", dev->name);
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
    char *DEVICE_TYPE_STRING[9] = {
        [DEVICE_UNKNOWN] = "UNKNOWN",
        [DEVICE_BLOCK] = "BLOCK",
        [DEVICE_CHAR] = "CHAR",
        [DEVICE_TTY] = "TTY",
        [DEVICE_NET] = "NET",
        [DEVICE_VIDEO] = "VIDEO",
        [DEVICE_TIMER] = "TIMER",
        [DEVICE_PSEUDO] = "PSEUDO",
        [DEVICE_VIRTUAL] = "VIRTUAL",
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

            uint64_t major = MAJOR(dev->device_id);
            uint64_t device = DEVICE(dev->device_id);
            uint64_t minor = MINOR(dev->device_id);

            log_info(MODULE, "device: %u, id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, read:%p, write:%p", i, dev->id, major, device, minor, DEVICE_TYPE_STRING[dev->type], dev->type, dev->name, dev->read, dev->write);
            dev = dev->next;
        }
    }
}
