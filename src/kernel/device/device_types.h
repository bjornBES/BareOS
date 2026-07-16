/*
 * File: device_types.h
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define DEVICE_NAME_MAX 32

typedef struct tty_dev_ops tty_dev_ops_t;

typedef enum device_type
{
    DEVICE_UNKNOWN = 0,
    DEVICE_CHAR = 1,   // keyboards, serial, mice — byte stream
    DEVICE_BLOCK = 2,  // disks, partitions — random access blocks
    DEVICE_TTY = 3,    // TTY devices
    DEVICE_NET = 4,    // network interfaces
    DEVICE_VIDEO = 5,  // framebuffer
    DEVICE_TIMER = 6,  // HPET, PIT, LAPIC
    DEVICE_PSEUDO = 7, // /dev/null, /dev/zero, /dev/random
    DEVICE_TYPE_MAX,
} device_type_t;

typedef struct device
{
    char name[DEVICE_NAME_MAX]; // built on register
    const char *class_name;     // set by driver
    device_type_t type;
    dev_t device_id;
    uint32_t flags;

    size_t (*read)(void *buf, off_t offset, size_t count, struct device *dev);
    size_t (*write)(void *buf, off_t offset, size_t count, struct device *dev);
    int (*ioctl)(uint32_t cmd, void *arg, struct device *dev);
    void (*destroy)(struct device *dev); // cleanup
    void (*tty_ops)(struct device *dev, tty_dev_ops_t *out);

    // private driver data
    void *priv;

    // intrusive list — for device registry
    struct device *next;
} device_t;