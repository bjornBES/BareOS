#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// MBR constants
#define SECTOR_SIZE 512
#define PARTITION_TABLE_OFFSET 0x1be
#define MBR_SIGNATURE_OFFSET 510

// Partition entry structure (16 bytes)
struct PartitionEntry
{
    uint8_t status;      // 0x80 = bootable, 0x00 = inactive
    uint8_t chsFirst[3]; // CHS address of first sector
    uint8_t type;        // Partition type (0x83 = Linux, 0x0B = FAT32, etc.)
    uint8_t chsLast[3];  // CHS address of last sector
    uint32_t lbaFirst;   // LBA of first sector
    uint32_t sectors;    // Number of sectors
};

int create_partition_table(const char *target, uint32_t start, uint32_t size, uint8_t type, int index, uint8_t bootable)
{
    FILE *f = fopen(target, "r+b");
    if (!f)
    {
        perror("open");
        return 1;
    }

    uint8_t mbr[SECTOR_SIZE];
    const size_t ret_code = fread(mbr, 1, SECTOR_SIZE, f);
    if (ret_code != SECTOR_SIZE)
    {
        printf("Only Read %u out of %u\n", ret_code, SECTOR_SIZE);
    }
    struct PartitionEntry* lp = (struct PartitionEntry*)mbr + PARTITION_TABLE_OFFSET;

    // printf("mbr partition index 0 lbaFirst:%u, sectors:%u, status:%x, type:%x\n", lp->lbaFirst, lp->sectors, lp->status, lp->type);
    struct PartitionEntry p;
    memset(&p, 0, sizeof(p));
    p.status = bootable;
    p.type = type;
    p.lbaFirst = start;
    p.sectors = size;
    // printf("mbr partition index %u mbr partition address %p\n", index, PARTITION_TABLE_OFFSET + index * 16);
    memcpy(&mbr[PARTITION_TABLE_OFFSET + index * 16], &p, sizeof(p));
    // printf("mbr partition index 0 lbaFirst:%u, sectors:%u, status:%x, type:%x\n", p.lbaFirst, p.sectors, p.status, p.type);

    fseek(f, 0, SEEK_SET);
    fwrite(mbr, 1, SECTOR_SIZE, f);
    fclose(f);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        printf("Usage: %s <disk> <start> <size> <type> <index> <bootable>\n", argv[0]);
        return 1;
    }

    const char *target = argv[1];
    uint32_t start = atoi(argv[2]);
    uint32_t size = atoi(argv[3]);
    uint8_t type = (uint8_t)strtol(argv[4], NULL, 16);
    int index = atoi(argv[5]);
    uint8_t bootable = atoi(argv[6]) ? 0x80 : 0x00;

    return create_partition_table(target, start, size, type, index, bootable);
}
