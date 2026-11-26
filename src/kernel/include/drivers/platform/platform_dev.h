/*
 * File: platform_dev.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "platform_driver.h"

#include "irq/irqt.h"
#include "dev/devicet.h"

#include "resource/resourcet.h"

#include <types.h>

#define PLATFORM_DEVID_NONE	(-1)
#define PLATFORM_DEVID_AUTO	(-2)

typedef struct platform_driver platform_driver_t;

typedef struct platform_dev
{
    device_t dev;
    int id;
    const char *name;

    interrupt_vector_t irq;

    uint32_t num_resources;
    resource_t *resource;

    void *platform_data;

    size_t irq_count;
    irq_range_t irq_range;
    kernel_irq_t kernel_irq;

    platform_driver_t *driver;

    void *priv;
} platform_device_t;

#define to_platform_dev(n) container_of(n, platform_device_t, dev)
