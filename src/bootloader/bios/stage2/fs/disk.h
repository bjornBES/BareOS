/*
 * File: disk.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
    bool have_extensions;
} DISK;

bool DISK_Initialize(DISK* disk, uint8_t driveNumber);
bool DISK_ReadSectors(DISK* disk, uint64_t lba, uint8_t sectors, void* lowerDataOut);
void DISK_LBA2CHS(DISK* disk, uint64_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut);
