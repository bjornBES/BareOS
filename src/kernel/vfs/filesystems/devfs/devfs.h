/*
 * File: devfs.h
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dev/devicet.h"

#include "vfs/vfs_internals.h"

#include "lists/hasht.h"

#include <types.h>

status_t devfs_add(device_t *dev);
