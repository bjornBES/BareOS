/*
 * File: fat.h
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs/vfs_internals.h"

#include "block/blkdevt.h"

#include <defs.h>
#include <types.h>

#define FAT_CACHE_SIZE    5
#define FAT12_EOC         0xFF8
#define FAT16_EOC         0xFFF8
#define FAT32_EOC         0x0FFFFFF8
#define FAT_CACHE_INVALID 0xFFFFFFFF

#define COMBINE_CLUSTERS(entry) ((entry).first_cluster_high << 16) | (entry).first_cluster_low

typedef uint32_t cluster_t;


typedef union fat_date
{
    uint16_t raw;
    struct
    {
        uint16_t year : 7;
        uint16_t month : 4;
        uint16_t day : 5;
    };
} __attribute__((packed)) fat_date_t;

typedef union fat_time
{
    uint16_t raw;
    struct
    {
        uint16_t hour : 5;
        uint16_t min : 6;
        uint16_t sec : 5;
    };
} __attribute__((packed)) fat_time_t;

typedef enum fat_attributes
{
    FAT_ATTRIBUTE_READ_ONLY = 0x01,
    FAT_ATTRIBUTE_HIDDEN = 0x02,
    FAT_ATTRIBUTE_SYSTEM = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID = 0x08,
    FAT_ATTRIBUTE_DIRECTORY = 0x10,
    FAT_ATTRIBUTE_ARCHIVE = 0x20,
    FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
} fat_attributes_t;

typedef struct fat_directory_entry
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
} PACKED fat_directory_entry_t;

typedef struct fat_long_directory_entry
{
    uint8_t order;
    int16_t name1[5];
    uint8_t attribute;
    uint8_t long_entry_type;
    uint8_t checksum;
    int16_t name2[6];
    uint16_t res;
    int16_t name3[2];
} PACKED fat_long_directory_entry_t;

typedef union fat_entry
{
    fat_directory_entry_t entry;
    fat_long_directory_entry_t long_entry;
} fat_entry_t;

typedef struct fat_directory
{
    fat_entry_t *entries;
    uint32_t entry_count;
} fat_directory_t;

typedef enum fat_type
{
    FAT12,
    FAT16,
    FAT32,
} fat_type_t;

typedef struct
{
    fat_type_t type;

    uint32_t sectors_per_cluster;
    uint32_t bytes_per_cluster;
    uint32_t reserved_sectors;

    cluster_t root_cluster;
    uint32_t root_start_lba;
    uint32_t root_sector_count;

    uint32_t data_start_lba;

    uint32_t fat_start_lba;
    uint8_t fat_count;

    uint32_t sectors_per_fat;
    uint16_t bytes_per_sector;

    uint8_t fat_cache[FAT_CACHE_SIZE * SECTOR_SIZE];
    uint32_t fat_cache_position;

    cluster_t total_clusters;

} fat_priv_data_t;


typedef struct fat_extended_boot_record
{
    // extended boot record
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;       // serial number, value doesn't matter
    uint8_t volume_label[11]; // 11 bytes, padded with spaces
    uint8_t system_id[8];
} PACKED fat_extended_boot_record_t;

typedef struct fat32_extended_boot_record
{
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_directory_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    fat_extended_boot_record_t ebr;
} PACKED fat32_extended_boot_record_t;

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
        fat_extended_boot_record_t ebr;
        fat32_extended_boot_record_t ebr32;
    };

    // ... we don't care about code ...

} PACKED fat_boot_sector_t;

typedef struct fat_inode
{
    inode_t base; // MUST be first
    cluster_t first_cluster;
    cluster_t current_cluster;
    cluster_t *cluster_chain;
    size_t chain_length;

    uint8_t created_time_tenths;
    fat_time_t created_time;
    fat_date_t created_date;
    fat_date_t accessed_date;
    fat_time_t modified_time;
    fat_date_t modified_date;
} fat_inode_t;

status_t fat_read_sectors(void *buffer, sector_t sector, size_t sector_count, device_t *dev, fat_priv_data_t *sb, ssize_t *out);
status_t fat_read_clusters(void *buffer, cluster_t cluster_start, cluster_t clusters_count, device_t *dev, fat_priv_data_t *sb, ssize_t *out);

status_t fat_83_to_name(const char raw83[11], char out[13]);
status_t fat_name_to_83(const char *name, uint8_t out83[11]);
status_t fat_name_eq(char name1[13], char name2[13]);

status_t fat_entry_find(fat_inode_t *parent, const char *entry_name, inode_t *entry_out, device_t *dev, mountpoint_t *mnt, fat_priv_data_t *sb);
