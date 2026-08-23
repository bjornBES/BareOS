/*
 * File: gpt.h
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include "partition.h"
#include "fs/disk.h"
#include "mbr.h"

int gpt_detect_partition(partition_t *part, disk_t *disk, mbr_entry_t *entry, int boot_partition_idx);