/*
 * File: fat.h
 * File Created: 18 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "partition/partition.h"
#include <defs.h>   

typedef struct
{
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
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

typedef struct
{
    uint8_t order;
    int16_t chars1[5];
    uint8_t attribute;
    uint8_t long_entry_type;
    uint8_t checksum;
    int16_t chars2[6];
    uint16_t _always_zero;
    int16_t chars3[2];
} PACKED fat_long_file_entry_t;

#define FAT_LFN_LAST 0x40

typedef struct
{
    int handle;
    bool is_directory;
    uint32_t position;
    uint32_t size;
} fat_file_t;

typedef struct
{
    bool is_directory;
} fat_stat_t;

enum fat_attributes_t
{
    FAT_ATTRIBUTE_READ_ONLY = 0x01,
    FAT_ATTRIBUTE_HIDDEN = 0x02,
    FAT_ATTRIBUTE_SYSTEM = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID = 0x08,
    FAT_ATTRIBUTE_DIRECTORY = 0x10,
    FAT_ATTRIBUTE_ARCHIVE = 0x20,
    FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

int fat_initialize(partition_t *part);
fat_file_t *fat_open(partition_t *part, const char *path);
bool fat_exist(partition_t *part, const char *path);
int fat_read(partition_t *part, fat_file_t *file, uint32_t byte_count, void *data_out);
void fat_close(partition_t *part, fat_file_t *file);
int fat_skip(partition_t *part, fat_file_t *file, uint32_t bytes);
int fat_seek(partition_t *part, fat_file_t *file, uint32_t position);
