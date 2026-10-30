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

#include "drivers/base/drivert.h"
#include "drivers/base/bust.h"
#include "drivers/base/classt.h"

#include <types.h>
#include <defs.h>
#include <binary.h>

#define DEVICE_NAME_MAX 32

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
    DEV_FLAG_OFFLINE_DISABLED = 8,
    DEV_FLAG_OFFLINE = 9,
    DEV_FLAG_NO_DRIVER = 10,
    DEV_FLAG_SAME_NAME = 11,

    DEV_FLAG_COUNT
} device_flags_t;

typedef struct device device_t;

typedef struct device_ops
{
    status_t (*read)(device_t *dev, void *buf, size_t len, off_t off, ssize_t *size_out);
    status_t (*write)(device_t *dev, const void *buf, size_t len, off_t off, ssize_t *size_out);
    /// @brief TODO
    status_t (*poll)(device_t *dev);
    // int     (*mmap)(device_t *dev, struct vm_area *vma);   // for framebuffers, DMA regions exposed to userspace

    status_t (*destroy)(device_t *dev); // cleanup
    // void (*tty_ops)(device_t *dev, tty_dev_ops_t *out);
} device_ops_t;
typedef struct device
{
    LIST_ENTRY_THIS();
    char name[DEVICE_NAME_MAX]; // built on register
    const char *init_name;      // set by driver

    void *bus_id;

    uint32_t id;
    device_id_t devt;       // opaque, generic-assigned
    struct device *parent;
    device_ops_t ops;

    bus_type_t *bus;

    driver_t *driver; // bound driver, if any
    void *drv_data;   // driver-private state, opaque to generic code

    dev_class_t *class;
    void *class_data;

    uint32_t flags;

    list_t children;
    list_node_t *siblings;
} device_t;

THIS_TO_TYPE_DEF(device_t);

#define CREATE_DEV_FLAG_ACCESSORS(accessor_name, flag_name)                  \
    ALWAYS_INLINE bool dev_##accessor_name(device_t *dev)                    \
    {                                                                        \
        return BIT_GET(dev->flags, flag_name) == 1;                          \
    }                                                                        \
    ALWAYS_INLINE void dev_set_##accessor_name(device_t *dev)                \
    {                                                                        \
        BIT_SET(dev->flags, flag_name);                                      \
    }                                                                        \
    ALWAYS_INLINE void dev_clear_##accessor_name(device_t *dev)              \
    {                                                                        \
        BIT_UNSET(dev->flags, flag_name);                                    \
    }                                                                        \
    ALWAYS_INLINE void dev_assign_##accessor_name(device_t *dev, bool value) \
    {                                                                        \
        BIT_ASSIGN(dev->flags, flag_name, value);                            \
    }                                                                        \
    ALWAYS_INLINE bool dev_test_and_set_##accessor_name(device_t *dev)       \
    {                                                                        \
        return BIT_TEST_AND_SET(dev->flags, flag_name);                      \
    }

CREATE_DEV_FLAG_ACCESSORS(ready_to_probe, DEV_FLAG_READY_TO_PROBE);
CREATE_DEV_FLAG_ACCESSORS(offline_disabled, DEV_FLAG_OFFLINE_DISABLED);
CREATE_DEV_FLAG_ACCESSORS(offline, DEV_FLAG_OFFLINE);
CREATE_DEV_FLAG_ACCESSORS(no_driver, DEV_FLAG_NO_DRIVER);
CREATE_DEV_FLAG_ACCESSORS(same_name, DEV_FLAG_SAME_NAME);

#undef CREATE_DEV_FLAG_ACCESSORS
