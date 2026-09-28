/*
 * File: dd.h
 * File Created: 27 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dev/device.h"

#include <types.h>

status_t device_attach(device_t *dev);
status_t device_initial_probe(device_t *dev);