/*
 * File: FAT.h
 * File Created: 09 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "VFS/vfs.h"

#define SECTOR_SIZE 512
#define FAT_CACHE_SIZE 5
#define FAT12_EOC 0xFF8
#define FAT16_EOC 0xFFF8
#define FAT32_EOC 0x0FFFFFF8
#define FAT_CACHE_INVALID 0xFFFFFFFF

typedef uint32_64 lba;
typedef uint32_t cluster;

typedef struct
{
    char name[11];
    uint8_t attributes;
    uint8_t res;
    uint8_t created_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed)) FAT_directory_entry;

typedef struct
{
    uint8_t order;
    int16_t name1[5];
    uint8_t attribute;
    uint8_t long_entry_type;
    uint8_t checksum;
    int16_t name2[6];
    uint16_t res;
    int16_t name3[2];
} __attribute__((packed)) FAT_long_directory_entry;

typedef union
{
    FAT_directory_entry entry;
    FAT_long_directory_entry long_entry;
} FAT_entry;

typedef struct
{
    FAT_entry *entries;
    uint32_t entry_count;
} FAT_directory;

typedef struct
{
    // extended boot record
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;       // serial number, value doesn't matter
    uint8_t volume_label[11]; // 11 bytes, padded with spaces
    uint8_t system_id[8];
} __attribute__((packed)) FAT_extended_boot_record;

typedef struct
{
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_directory_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    FAT_extended_boot_record ebr;

} __attribute((packed)) FAT_32_extended_boot_record;

typedef struct
{
    uint8_t boot_jump_instruction[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t directory_entry_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;

    union
    {
        FAT_extended_boot_record EBR1216;
        FAT_32_extended_boot_record EBR32;
    };

    // ... we don't care about code ...

} __attribute__((packed)) FAT_boot_sector;

typedef enum __FatType
{
    FAT12,
    FAT16,
    FAT32,
} fat_type;

enum FAT_Attributes
{
    FAT_ATTRIBUTE_READ_ONLY = 0x01,
    FAT_ATTRIBUTE_HIDDEN = 0x02,
    FAT_ATTRIBUTE_SYSTEM = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID = 0x08,
    FAT_ATTRIBUTE_DIRECTORY = 0x10,
    FAT_ATTRIBUTE_ARCHIVE = 0x20,
    FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

typedef struct
{
    fat_type type;

    uint32_t sectors_per_cluster;
    uint32_t bytes_per_cluster;
    uint32_t reserved_sectors;

    cluster root_cluster;
    uint32_t root_start_lba;
    uint32_t root_sector_count;

    uint32_t data_start_lba;

    uint32_t fat_start_lba;
    uint8_t fat_count;

    uint32_t sectors_per_fat;
    uint16_t bytes_per_sector;

    uint8_t fat_cache[FAT_CACHE_SIZE * SECTOR_SIZE];
    uint32_t fat_cache_position;

    cluster total_clusters;

} fat_priv_data;

typedef struct fat_inode
{
    inode_t base; // MUST be first
    cluster first_cluster;
    cluster current_cluster;
    cluster *cluster_chain;
    size_t chain_length;
} fat_inode_t;

void fat_init();