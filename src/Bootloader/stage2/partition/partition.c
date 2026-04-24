/*
 * File: partition.c
 * File Created: 17 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "partition.h"

#include "stdio.h"

bool Partition_ReadSectors(Partition* part, uint32_t lba, uint8_t sectors, void* lowerDataOut)
{
    uint16_t cylinder;
    uint16_t sector;
    uint16_t head;
    DISK_LBA2CHS(part->disk, lba, &cylinder, &sector, &head);
    printf("Reading | disk: %x, lba: %u, count %u CHS: %u|%u|%u\r\n", part->disk->id, lba + part->partitionOffset, sectors, cylinder, sector, head);
    return DISK_ReadSectors(part->disk, lba + part->partitionOffset, sectors, lowerDataOut);
}
