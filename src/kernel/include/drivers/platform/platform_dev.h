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

#include "irq/irqt.h"
#include "dev/devicet.h"

#include "resource/resourcet.h"

#include <types.h>

typedef struct platform_driver platform_driver_t;

typedef struct platform_dev
{
    const char *name;

    interrupt_vector_t irq;

    device_t dev;

    uint32_t num_resources;
    resource_t **resource;

    size_t irq_count;
    irq_range_t irq_range;
    kernel_irq_t kernel_irq;

    // resource_t resource[DEVICE_COUNT_RESOURCE];
    // resource_t driver_exclusive_resource;

    platform_driver_t *driver;

    void *priv;
} platform_device_t;

#define to_platform_dev(n) container_of(n, platform_device_t, dev)
