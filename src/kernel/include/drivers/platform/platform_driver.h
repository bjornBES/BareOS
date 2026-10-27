/*
 * File: platform_driver.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "platform_dev.h"

typedef struct platform_driver
{
    status_t (*probe)(platform_device_t *dev);
    void (*remove)(platform_device_t *dev);

    driver_t driver;
} platform_driver_t;

#define to_platform_driver(__drv)                                   \
    (__drv ? container_of(__drv, platform_driver_t, driver) : NULL)
