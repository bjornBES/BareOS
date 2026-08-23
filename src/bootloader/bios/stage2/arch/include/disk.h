/*
 * File: disk.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include "fs/disk.h"

int disk_arch_initialize(disk_t *disk, uint16_t boot_drive);