/*
 * File: blkdev.h
 * File Created: 15 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "blkdevt.h"

#include "dev/devicet.h"

#include <types.h>

status_t block_device_create(device_t **out_dev);
