/*
 * File: disk.h
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct disk
{
    uint16_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
    bool have_extensions;
} disk_t;

int disk_initialize(disk_t *disk, uint16_t boot_drive);
