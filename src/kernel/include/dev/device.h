/*
 * File: device.h
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "device_id.h"
#include <types.h>

#define DEVICE_NAME_MAX 32

struct device;

typedef status_t (*device_irq_handler_t)(struct device *dev);

typedef enum dev_class
{
    DEVICE_UNKNOWN = 0,
    DEVICE_CHAR,   // keyboards, serial, mice — byte stream
    DEVICE_SERIAL,
    DEVICE_BLOCK,  // disks, partitions — random access blocks
    DEVICE_TTY,    // TTY devices
    DEVICE_NET,    // network interfaces
    DEVICE_VIDEO,  // framebuffer
    DEVICE_TIMER,  // HPET, PIT, LAPIC
    DEVICE_PSEUDO, // /dev/null, /dev/zero, /dev/random
    DEVICE_VIRTUAL,
    DEVICE_TYPE_MAX,
} dev_class_t;


typedef struct driver
{
    const char *name;  // "ahci", "rtl8139", etc.
    dev_class_t class; // what kind of device this driver handles

    // const struct dev_match *match_table; // array of {vendor, device, class} to match against
    size_t match_count;

    status_t (*probe)(struct device *dev);   // called when a match is found, driver decides accept/reject
    void (*remove)(struct device *dev);      // hot-unplug / teardown
    status_t (*suspend)(struct device *dev); // optional, power state transitions
    status_t (*resume)(struct device *dev);

    // struct list_node    node;         // linkage into the global driver registry
} driver_t;

struct device_ops;

typedef struct device
{
    char name[DEVICE_NAME_MAX]; // built on register
    char *device_name;          // the devices name
    const char *class_name;     // set by driver

    uint32_t id;
    device_id_t devt;             // opaque, generic-assigned
    struct device *parent;
    struct device_ops *ops;     // vtable: probe, read, write, ioctl, remove...
    driver_t *driver;           // bound driver, if any
    void *drv_data;             // driver-private state, opaque to generic code
    dev_class_t class;
    uint32_t flags;
    
    struct device *siblings;

    // struct list_node  children;
    // struct list_node  siblings;
} device_t;

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

device_t *device_create();
status_t device_register_irq(device_t *dev, device_irq_handler_t generic_handler);

uint32_t device_id_get_next_class_id(const char *class_name);
uint32_t device_id_get_id(device_t *dev);


status_t device_register(device_t *dev);
void device_debug();