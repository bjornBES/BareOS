/*
 * File: devfs.h
 * File Created: 16 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "device/device_types.h"

void devfs_create_entry(device_t *dev, int flags);
void devfs_init();