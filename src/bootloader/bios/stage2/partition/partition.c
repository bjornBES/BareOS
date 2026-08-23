/*
 * File: partition.c
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "partition.h"

#include "mbr.h"
#include "gpt.h"

#include "stdio.h"

int partition_detect(partition_t *part, disk_t *disk, void *partition_data, int boot_partition_idx)
{
    mbr_entry_t **entries = &partition_data;
    if ((entries[0]->attributes & 0x80) == 0x00 && entries[0]->partitionType == 0xEE)
    {
        return gpt_detect_partition(part, disk, entries[0], boot_partition_idx);
    }

    mbr_entry_t *boot_entry = entries[boot_partition_idx];

    part->disk = disk;
    part->partition_offset = boot_entry->lbaStart;
    part->partition_size = boot_entry->size;
    for (int i = 0; i < 4; i++)
    {
        mbr_entry_t *entry = partition_data + i * sizeof(mbr_entry_t);
        if (entry->size == 0)
        {
            continue;
        }
        printf("Got partition %u @ 0x%p\n", i, entry);
        printf("      attributes 0x%x\n", entry->attributes);
        printf("       chs start 0x%x%x%x\n", entry->chsStart[2], entry->chsStart[1], entry->chsStart[0]);
        printf("  partition type 0x%x\n", entry->partitionType);
        printf("         chs end 0x%x%x%x\n", entry->chsEnd[2], entry->chsEnd[1], entry->chsEnd[0]);
        printf("       lba start 0x%x\n", entry->lbaStart);
        printf("            size 0x%x\n", entry->size);
    }
}
