/*
 * File: gpt.c
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "gpt.h"
#include "stdio.h"


int gpt_detect_partition(partition_t *part, disk_t *disk, mbr_entry_t *entry, int boot_partition_idx)
{
    printf("gpt_detect_partition(%p, %p, %p, %u)", part, disk, entry, boot_partition_idx);
    return 0;
}