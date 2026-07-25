/*
 * File: gpt.h
 * File Created: 17 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "fs/disk.h"
#include "partition.h"
#include "mbr.h"

#include <stdint.h>

void GPT_detect_partition(Partition* part, DISK* disk, MBR_entry* mbr_partition_entry);
