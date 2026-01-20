#include "mbr.h"
#include "memory.h"
#include "stdio.h"
#include "disk.h"
#include "memdefs.h"

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

} __attribute__((packed)) MBR_Entry;


void MBR_DetectPartition(Partition* part, DISK* disk, void* partition)
{
    // using segoffset_to_linear 0x1fe
    // 0x401be
    MBR_Entry* entry = (MBR_Entry*)(partition);
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

bool Partition_ReadSectors(Partition* part, uint32_t lba, uint8_t sectors, void* lowerDataOut)
{
    uint16_t cylinder;
    uint16_t sector;
    uint16_t head;
    DISK_LBA2CHS(part->disk, lba, &cylinder, &sector, &head);
    // printf("Reading | disk: %u, lba: %u, count %u CHS: %u|%u|%u\r\n", part->disk, lba + part->partitionOffset, sectors, cylinder, sector, head);
    return DISK_ReadSectors(part->disk, lba + part->partitionOffset, sectors, lowerDataOut);
}