/*
 * File: mbr.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "fs/disk.h"
#include "partition.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    // 0x00	1	Drive attributes (bit 7 set = active or bootable)
    uint8_t attributes;

    // 0x01	3	CHS Address of partition start
    uint8_t chsStart[3];

    // 0x04	1	Partition type
    uint8_t partitionType;

    // 0x05	3	CHS address of last partition sector
    uint8_t chsEnd[3];

    // 0x08	4	LBA of partition start
    uint32_t lbaStart;

    // 0x0C	4	Number of sectors in partition
    uint32_t size;

} __attribute__((packed)) MBR_entry;

void MBR_detect_partition(Partition* part, DISK* disk, void* partitionEntry);

bool Partition_ReadSectors(Partition* disk, uint32_t lba, uint8_t sectors, void* lowerDataOut);
