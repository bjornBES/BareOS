/*
 * File: FAT.h
 * File Created: 09 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "VFS/vfs.h"

#define SECTOR_SIZE             512
#define FAT_CACHE_SIZE          5
#define FAT12_EOC               0xFF8
#define FAT16_EOC               0xFFF8
#define FAT32_EOC               0x0FFFFFF8
#define FAT_CACHE_INVALID       0xFFFFFFFF

typedef struct 
{
    char Name[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) FAT_directory_entry;

typedef struct 
{
    uint8_t Order;
    int16_t name1[5];
    uint8_t Attribute;
    uint8_t LongEntryType;
    uint8_t Checksum;
    int16_t name2[6];
    uint16_t _AlwaysZero;
    int16_t name3[2];
} __attribute__((packed)) FAT_long_directory_entry;

typedef union
{
    FAT_directory_entry entry;
    FAT_long_directory_entry long_entry;
} FAT_entry;

typedef struct
{
    FAT_entry entries[32]; // update this at some point
    uint32_t entry_count;
} FAT_directory;

typedef struct 
{
    // extended boot record
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;          // serial number, value doesn't matter
    uint8_t volume_label[11];    // 11 bytes, padded with spaces
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

    union {
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
    FAT_ATTRIBUTE_READ_ONLY         = 0x01,
    FAT_ATTRIBUTE_HIDDEN            = 0x02,
    FAT_ATTRIBUTE_SYSTEM            = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID         = 0x08,
    FAT_ATTRIBUTE_DIRECTORY         = 0x10,
    FAT_ATTRIBUTE_ARCHIVE           = 0x20,
    FAT_ATTRIBUTE_LFN               = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

typedef struct {
    fat_type type;

    uint32_t sectors_per_cluster;
    uint32_t bytes_per_cluster;
    uint32_t reserved_sectors;

    uint32_t root_cluster;
    uint32_t root_start_lba;
    uint32_t root_sector_count;

    uint32_t data_start_lba;

    uint32_t fat_start_lba;
    uint8_t  fat_count;

    uint32_t sectors_per_fat;
    uint16_t bytes_per_sector;

    uint8_t fat_cache[FAT_CACHE_SIZE * SECTOR_SIZE];
    uint32_t fat_cache_position;

} fat_priv_data;

typedef struct {


    // for function fat_read_directory
    
} fat_priv_node_data;

filesystem *fat_init();