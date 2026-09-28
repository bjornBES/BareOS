/*
 * File: devicet.h
 * File Created: 27 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "device_id.h"
#include "drivers/drivert.h"
#include "drivers/bust.h"

#include <types.h>
#include <defs.h>
#include <binary.h>

#define DEVICE_NAME_MAX 32

typedef enum dev_type
{
    DEVICE_UNKNOWN = 0,
    DEVICE_CHAR,   // keyboards, serial, mice — byte stream
    DEVICE_SERIAL,
    DEVICE_BLOCK,  // disks, partitions — random access blocks
    DEVICE_TTY,    // TTY devices
    DEVICE_NET,    // network interfaces
    DEVICE_VIDEO,  // framebuffer
    DEVICE_TIMER,
    DEVICE_PSEUDO, // /dev/null, /dev/zero, /dev/random
    DEVICE_VIRTUAL,
    DEVICE_TYPE_MAX,
} dev_type_t;

typedef enum device_flags
{
    DEV_FLAG_READY_TO_PROBE = 0,
    // DEV_FLAG_CAN_MATCH = 1,
    // DEV_FLAG_DMA_IOMMU = 2,
    // DEV_FLAG_DMA_SKIP_SYNC = 3,
    // DEV_FLAG_DMA_OPS_BYPASS = 4,
    // DEV_FLAG_STATE_SYNCED = 5,
    // DEV_FLAG_DMA_COHERENT = 6,
    // DEV_FLAG_OF_NODE_REUSED = 7,
    // DEV_FLAG_OFFLINE_DISABLED = 8,
    DEV_FLAG_OFFLINE = 9,

    DEV_FLAG_COUNT
} device_flags_t;

struct device_ops;

typedef struct device
{
    char name[DEVICE_NAME_MAX]; // built on register
    char *device_name;          // the devices name
    const char *init_name;      // set by driver

    void *bus_id;

    uint32_t id;
    device_id_t devt;       // opaque, generic-assigned
    struct device *parent;
    struct device_ops *ops; // vtable: probe, read, write, ioctl, remove...

    bus_type_t *bus;

    driver_t *driver; // bound driver, if any
    void *drv_data;   // driver-private state, opaque to generic code
    dev_type_t class;
    uint32_t flags;

    struct device *siblings;

    // struct list_node  children;
    // struct list_node  siblings;
} device_t;

#define CREATE_DEV_FLAG_ACCESSORS(accessor_name, flag_name)                  \
    ALWAYS_INLINE bool dev_##accessor_name(device_t *dev)                    \
    {                                                                        \
        return FLAG_GET(dev->flags, flag_name) == flag_name;                 \
    }                                                                        \
    ALWAYS_INLINE void dev_set_##accessor_name(device_t *dev)                \
    {                                                                        \
        FLAG_SET(dev->flags, flag_name);                                     \
    }                                                                        \
    ALWAYS_INLINE void dev_clear_##accessor_name(device_t *dev)              \
    {                                                                        \
        FLAG_UNSET(dev->flags, flag_name);                                   \
    }                                                                        \
    ALWAYS_INLINE void dev_assign_##accessor_name(device_t *dev, bool value) \
    {                                                                        \
        FLAG_ASSIGN(dev->flags, flag_name, value);                           \
    }                                                                        \
    ALWAYS_INLINE bool dev_test_and_set_##accessor_name(device_t *dev)       \
    {                                                                        \
        return FLAG_TEST_AND_SET(dev->flags, DEV_FLAG_READY_TO_PROBE);       \
    }

CREATE_DEV_FLAG_ACCESSORS(ready_to_probe, DEV_FLAG_READY_TO_PROBE);

#undef CREATE_DEV_FLAG_ACCESSORS

struct device_ops
{
    ssize_t (*read)(device_t *dev, void *buf, size_t len, off_t off);
    ssize_t (*write)(device_t *dev, const void *buf, size_t len, off_t off);
    status_t (*ioctl)(device_t *dev, unsigned long cmd, void *arg);
    // int     (*mmap)(device_t *dev, struct vm_area *vma);   // for framebuffers, DMA regions exposed to userspace

    status_t (*open)(device_t *dev);
    status_t (*close)(device_t *dev);

    void (*destroy)(device_t *dev); // cleanup
    // void (*tty_ops)(device_t *dev, tty_dev_ops_t *out);

    status_t (*irq_handler)(device_t *dev); // called by generic IRQ dispatch, not by arch code directly
};
