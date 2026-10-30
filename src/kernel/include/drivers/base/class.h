/*
 * File: class.h
 * File Created: 15 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "classt.h"

status_t class_add_device(device_t *dev);
status_t class_remove_device(device_t *dev);

status_t class_list_devices();

status_t class_register(dev_class_t *class);
