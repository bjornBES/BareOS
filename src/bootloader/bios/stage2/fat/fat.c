/*
 * File: fat.c
 * File Created: 18 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fat.h"
#include "config.h"

#define MAX_PATH_SIZE CONFIG_MAX_PATH_LENGTH

#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 4
#define ROOT_DIRECTORY_HANDLE -1
#define FAT_CACHE_SIZE 5

typedef struct
{
    // extended boot record
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;       // serial number, value doesn't matter
    uint8_t volume_label[11]; // 11 bytes, padded with spaces
    uint8_t system_id[8];
} __attribute__((packed)) fat_extended_boot_record_t;

typedef struct
{
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_directory_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    fat_extended_boot_record_t ebr;

} __attribute((packed)) fat32_extended_boot_record_t;

typedef struct
{
    uint8_t boot_jump_instruction[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t dir_entry_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;

    union
    {
        fat_extended_boot_record_t ebr1216;
        fat32_extended_boot_record_t ebr32;
    };

    // ... we don't care about code ...

} __attribute__((packed)) fat_boot_sector_t;

typedef struct
{
    uint8_t buffer[SECTOR_SIZE];
    fat_file_t public;
    bool opened;
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector_in_cluster;

} fat_file_data_t;

typedef struct
{
    uint8_t order;
    int16_t chars[13];
} fat_lfn_block_t;


typedef struct
{
    union
    {
        fat_boot_sector_t boot_sector;
        uint8_t boot_sector_bytes[SECTOR_SIZE];
    };

    fat_file_data_t root_directory;

    fat_file_data_t opened_files[MAX_FILE_HANDLES];

    uint8_t fat_cache[FAT_CACHE_SIZE * SECTOR_SIZE];
    uint32_t fat_cache_position;

    fat_lfn_block_t lfn_blocks[FAT_LFN_LAST];
    int lfn_count;

} fat_data;

int fat_initialize(partition_t *part)
{

}

fat_file_t *fat_open(partition_t *disk, const char *path)
{

}

uint32_t fat_read(partition_t *disk, fat_file_t *file, uint32_t byte_count, void *data_out)
{

}

int fat_read_entry(partition_t *disk, fat_file_t *file, fat_directory_entry_t *dir_entry)
{

}

void fat_close(fat_file_t *file)
{

}

int fat_skip(partition_t *part, fat_file_t *file, uint32_t bytes)
{

}

int fat_seek(partition_t *part, fat_file_t *file, uint32_t position)
{

}
