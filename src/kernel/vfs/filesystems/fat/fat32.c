/*
 * File: fat32.c
 * File Created: 19 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fat.h"

cluster_t fat32_next_cluster(cluster_t current_cluster, device_t *dev, fat_priv_data_t *sb)
{
    // ENTER_FUNC(MODULE, "%u, %p, %p", current_cluster, dev, sb);
    uint32_t fat_index;

    fat_index = current_cluster * 4;

    sector_t fat_index_sector = fat_index / sb->bytes_per_sector;
    if (fat_index_sector < sb->fat_cache_position || fat_index_sector >= sb->fat_cache_position + FAT_CACHE_SIZE)
    {
        ssize_t _;
        dev->ops.read(dev, sb->fat_cache, FAT_CACHE_SIZE, sb->fat_start_lba + fat_index_sector, &_);
        sb->fat_cache_position = fat_index_sector;
    }

    fat_index -= sb->fat_cache_position * sb->bytes_per_sector;

    cluster_t next_cluster;

    next_cluster = *(uint32_t *)(sb->fat_cache + fat_index) & 0x0FFFFFFF;

    if (next_cluster >= FAT32_EOC)
    {
        return 0xFFFFFFFF;
    }

    return next_cluster;
}
