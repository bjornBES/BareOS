/*
 * File: disk.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "disk.h"
#include "x86.h"
#include "stdio.h"
#include "memory.h"
#include "bios/bios.h"

static extensions_dap __attribute__((aligned(16))) s_dap;

bool DISK_Initialize(DISK *disk, uint8_t driveNumber)
{
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    if (!x86_Disk_GetDriveParams(driveNumber, &driveType, &cylinders, &sectors, &heads))
        return false;

    disk->id = driveNumber;
    if (driveNumber >= 0x80)
    {
        disk->have_extensions = x86_ExtensionSupport(driveNumber);
        printf("disk %x has Extension Support? %u\n", disk->id, disk->have_extensions);
    }

    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;

    return true;
}

void DISK_LBA2CHS(DISK *disk, uint64_t lba, uint16_t *cylinderOut, uint16_t *sectorOut, uint16_t *headOut)
{
    // sector = (LBA % sectors per track + 1)
    *sectorOut = lba % disk->sectors + 1;

    // cylinder = (LBA / sectors per track) / heads
    *cylinderOut = (lba / disk->sectors) / disk->heads;

    // head = (LBA / sectors per track) % heads
    *headOut = (lba / disk->sectors) % disk->heads;
}

extern void hexdump(void *ptr, int len);
bool DISK_ReadSectors(DISK *disk, uint64_t lba, uint8_t sectors, void *dataOut)
{
    uint16_t cylinder, sector, head;

    for (int i = 0; i < 3; i++)
    {
        int result;
        if (disk->have_extensions)
        {
            memset(dataOut, 0xAA, 512);
            seg_off_t memory = linear_to_segoff(dataOut);
            extensions_dap *dap = (extensions_dap *)&s_dap;
            dap->size = 0x10;
            dap->count = sectors;
            dap->memory = memory;
            dap->lba = lba;
            x86_Disk_Read_Extended(disk->id, dap);
            printf("%u = x86_Disk_Read(disk->id:%x, count: %u, memory: %x:%x(0x%p), %u)\n", result, disk->id, sectors, memory.segment, memory.offset, dataOut, lba);
            // hexdump((void *)dataOut, 512);
        }
        else
        {
            DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);
            printf("x86_Disk_Read(disk->id:%x, cylinder:%u, sector:%u, head:%u, sectors:%u, dataOut:%p)\n", disk->id, cylinder, sector, head, sectors, dataOut);
            result = x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut);
        }
        // printf("result = %u\n", result);
        if (!result)
        {
            return true;
        }

        x86_Disk_Reset(disk->id);
    }

    return false;
}
