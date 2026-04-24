/*
 * File: gpt.c
 * File Created: 17 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "gpt.h"

#include "stdio.h"
#include "memory.h"

#define MAGIC "EFI PART"

typedef union
{
    struct
    {
        uint32_t time_low;
        uint16_t time_mid;
        uint16_t time_high_and_version;
        uint8_t click_high_and_res;
        uint8_t clock_low;
        uint8_t node[6];
    };
    uint8_t raw[16];
} uuid;


typedef struct {
    uuid part_UUID;
    uuid UUID;
    uint64_t start_lba;
    uint64_t end_lba;
    uint64_t attributes;
    uint16_t partition_name[36];
} __attribute__((packed)) GPT_entry;

typedef union
{
    struct
    {
        char magic[8];
        uint32_t gpt_version;
        uint32_t header_size;
        uint32_t crc32_header;
        uint32_t res;
        uint64_t header_lba;
        uint64_t backup_header_lba;
        uint64_t first_usable_lba;
        uint64_t last_usable_lba;
        uuid drive_uuid;
        uint64_t lba_part_entries;
        uint32_t part_entry_count;
        uint32_t part_entries_size;
        uint32_t crc32_part_entries;
    };
    uint8_t raw[512];
} GPT_header;


void GPT_detect_partition(Partition* part, DISK* disk, MBR_entry* mbr_partition_entry)
{
    GPT_header header;
    DISK_ReadSectors(disk, mbr_partition_entry->lbaStart, 1, &header);

    if (memcmp(header.magic, MAGIC, 8))
    {
        printf("GPT Header not valid");
        return;
    }

    printf("got header done");
}
