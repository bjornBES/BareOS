/*
 * File: seriot.h
 * File Created: 26 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/base/core_types.h"

#include "sync/spinlock.h"
#include "sync/mutext.h"

#include "lists/list_node.h"

#include "dev/devicet.h"

#include <types.h>

#define SERIO_ANY 0xFF

typedef struct serio_device_id
{
    uint8_t type;
    uint8_t extra;
    uint8_t id;
    uint8_t proto;
} serio_device_id_t;

struct serio_driver;

typedef struct serio_dev
{
    void *port_data;

    char name[32];
    // char phys[32];
    // char firmware_id[128];

    bool manual_bind;

    serio_device_id_t id;

    spinlock_t lock;

    status_t (*write)(struct serio_dev *serio_dev, unsigned char);
    status_t (*open)(struct serio_dev *serio_dev);
    status_t (*close)(struct serio_dev *serio_dev);
    status_t (*start)(struct serio_dev *serio_dev);
    status_t (*stop)(struct serio_dev *serio_dev);

    struct serio_dev *parent;
    /* Entry in parent->children list */
    list_node_t child_node;
    list_node_t children;
    /* Level of nesting in serio hierarchy */
    unsigned int depth;

    /*
     * serio->drv is accessed from interrupt handlers; when modifying
     * caller should acquire serio->drv_mutex and serio->lock.
     */
    struct serio_driver *drv;

    /* Protects serio->drv so attributes can pin current driver */
    mutex_t drv_mutex;

    device_t dev;

    LIST_ENTRY_THIS();

    mutex_t *ps2_cmd_mutex;
} serio_dev_t;

#define to_serio_port(d) container_of(d, struct serio, dev)

typedef struct serio_driver
{
    const char *description;

    const serio_device_id_t *id_table;
    bool manual_bind;

    status_t (*write_wakeup)(struct serio_dev *serio_dev);
    status_t (*interrupt)(struct serio_dev *serio_dev, unsigned char, unsigned int);
    status_t (*connect)(struct serio_dev *serio_dev, struct serio_driver *drv);
    status_t (*reconnect)(struct serio_dev *serio_dev);
    status_t (*fast_reconnect)(struct serio_dev *serio_dev);
    status_t (*disconnect)(struct serio_dev *serio_dev);
    status_t (*cleanup)(struct serio_dev *serio_dev);

    driver_t driver;
} serio_driver_t;
