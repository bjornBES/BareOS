/*
 * File: platform.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "platform_driver.h"

#include "module.h"

status_t platform_device_register(platform_device_t *platform_dev);

status_t platform_register_driver(platform_driver_t *driver);
status_t platform_unregister_driver(platform_driver_t *driver);

#define MODULE_PLATFORM_DRIVER(__platform_driver)                                     \
    MODULE_DRIVER(__platform_driver, platform_register_driver, platform_unregister_driver)
