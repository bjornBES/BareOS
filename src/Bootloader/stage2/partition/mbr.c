/*
 * File: mbr.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mbr.h"
#include "gpt.h"
#include "memory.h"
#include "stdio.h"
#include "memdefs.h"


void MBR_detect_partition(Partition* part, DISK* disk, void* partition)
{
    // using segoffset_to_linear 0x1fe
    // 0x401be
    MBR_entry* entry = (MBR_entry*)(partition);
    printf("&entry = 0x%p\n", entry);
    printf("disk id = %x\n", disk->id);
    part->disk = disk;
    if (disk->id < 0x80)
    {
        part->partitionOffset = 0;
        part->partitionSize = (uint32_t)(disk->cylinders)
             * (uint32_t)(disk->heads)
             * (uint32_t)(disk->sectors);
    }
    else
    {
        if ((entry->attributes & 0x80) == 0x00 && entry->partitionType == 0xEE)
        {
            GPT_detect_partition(part, disk, entry);
        }
        part->partitionOffset = entry->lbaStart;
        part->partitionSize = entry->size;
        printf("Got parttition\n");
        printf("     attributes 0x%x\n", entry->attributes);
        printf("       chsStart 0x%x%x%x\n", entry->chsStart[2], entry->chsStart[1], entry->chsStart[0]);
        printf("  partitionType 0x%x\n", entry->partitionType);
        printf("         chsEnd 0x%x%x%x\n", entry->chsEnd[2], entry->chsEnd[1], entry->chsEnd[0]);
        printf("       lbaStart 0x%x\n", entry->lbaStart);
        printf("           size 0x%x\n", entry->size);
    }
}
