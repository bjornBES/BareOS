/*
 * File: FAT.c
 * File Created: 09 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "FAT.h"
#include "fat_name.h"

#include "VFS/vfs_flags.h"
#include "kernel.h"
#include "libs/malloc.h"
#include "libs/ctype.h"
#include "libs/stdio.h"
#include "libs/malloc.h"
#include "libs/memory.h"
#include "libs/string.h"

#include "debug/debug.h"

#define MODULE "FAT"

#define PRINT_FUNCTION_INFO(func, ...)              \
    fprintf(VFS_FD_DEBUG, "in function %s(", func); \
    fprintf(VFS_FD_DEBUG, __VA_ARGS__);             \
    fprintf(VFS_FD_DEBUG, ")\n");

#define COMBINE_CLUSTERS(entry) (entry.first_cluster_high << 16) | entry.first_cluster_low
extern void hexdump(void *ptr, int len);
static inline lba bytes_to_lba(size_t bytes, fat_priv_data *sb)
{
    return bytes / sb->bytes_per_sector;
}
static inline size_t lba_to_bytes(lba lba, fat_priv_data *sb)
{
    return lba * sb->bytes_per_sector;
}
static inline cluster bytes_to_cluster(size_t bytes, fat_priv_data *sb)
{
    lba lba = bytes / sb->bytes_per_sector;
    return ((lba - sb->data_start_lba) / sb->sectors_per_cluster) + 2;
}
static inline size_t cluster_to_bytes(cluster clusters, fat_priv_data *sb)
{
    lba l = sb->data_start_lba + (clusters - 2) * sb->sectors_per_cluster;
    return l * sb->bytes_per_sector;
}
static inline lba cluster_to_lba(cluster cluster, fat_priv_data *sb)
{
    return sb->data_start_lba + (cluster - 2) * sb->sectors_per_cluster;
}
static inline cluster lba_to_cluster(lba lba, fat_priv_data *sb)
{
    return ((lba - sb->data_start_lba) / sb->sectors_per_cluster) + 2;
}

size_t fat_read_sectors(void *buffer, lba sector, lba sector_count, device_t *dev, fat_priv_data *sb)
{
    log_debug(MODULE, "fat_read_sectors(%p, %u, %u, %p, %p)", buffer, sector, sector_count, dev, sb);
    return dev->read(buffer, sector, sector_count, dev) * sb->bytes_per_sector;
}
size_t fat_read_clusters(void *buffer, cluster cluster_start, cluster clusters_count, device_t *dev, fat_priv_data *sb)
{
    log_debug(MODULE, "fat_read_clusters(%p, %u, %u, %p, %p)", buffer, cluster_start, clusters_count, dev, sb);
    lba lba = cluster_to_lba(cluster_start, sb);
    size_t len = fat_read_sectors(buffer, lba, clusters_count * sb->sectors_per_cluster, dev, sb);
    return len;
}

uint32_t fat_write_sectors(void *buffer, lba sector, lba sector_count, device_t *dev, fat_priv_data *sb)
{
    return dev->write(buffer, sector, sector_count, dev) * sb->bytes_per_sector;
}
void fat_write_clusters(void *buffer, cluster cluster_start, cluster clusters_count, device_t *dev, fat_priv_data *sb)
{
    lba lba = cluster_to_lba(cluster_start, sb);
    fat_write_sectors(buffer, lba, clusters_count * sb->sectors_per_cluster, dev, sb);
}

bool fat_is_eoc(fat_priv_data *sb, uint32_t cluster)
{
    switch (sb->type)
    {
    case FAT12:
        return cluster >= FAT12_EOC;
    case FAT16:
        return cluster >= FAT16_EOC;
    case FAT32:
        return cluster >= FAT32_EOC;
    }
    return true;
}

cluster fat_next_cluster(cluster current_cluster, device_t *dev, fat_priv_data *sb)
{
    log_debug(MODULE, "fat_next_cluster(%u, %p, %p)", current_cluster, dev, sb);
    uint32_t fat_index;
    if (sb->type == FAT12)
    {
        fat_index = current_cluster * 3 / 2;
    }
    else if (sb->type == FAT16)
    {
        fat_index = current_cluster * 2;
    }
    else if (sb->type == FAT32)
    {
        fat_index = current_cluster * 4;
    }

    lba fat_index_sector = fat_index / sb->bytes_per_sector;
    if (fat_index_sector < sb->fat_cache_position || fat_index_sector >= sb->fat_cache_position + FAT_CACHE_SIZE)
    {
        dev->read(sb->fat_cache, sb->fat_start_lba + fat_index_sector, FAT_CACHE_SIZE, dev);
        sb->fat_cache_position = fat_index_sector;
    }

    fat_index -= sb->fat_cache_position * sb->bytes_per_sector;

    cluster next_cluster;

    if (sb->type == FAT12)
    {
        if (current_cluster % 2 == 0)
        {
            next_cluster = (*(uint16_t *)(sb->fat_cache + fat_index)) & 0x0FFF;
        }
        else
        {
            next_cluster = (*(uint16_t *)(sb->fat_cache + fat_index)) >> 4;
        }
    }
    else if (sb->type == FAT16)
    {
        next_cluster = *(uint16_t *)(sb->fat_cache + fat_index);
    }
    else if (sb->type == FAT32)
    {
        next_cluster = *(uint32_t *)(sb->fat_cache + fat_index) & 0x0FFFFFFF;
    }

    if (fat_is_eoc(sb, next_cluster))
    {
        return 0xFFFFFFFF;
    }

    return next_cluster;
}

size_t fat_read_file(vfs_node_t *node, void *buffer, off_t offset, size_t size, device_t *dev, mountpoint_t *mnt)
{
    log_debug(MODULE, "fat_read_file(%p, %p, %u, %u, %p, %p)", node, buffer, offset, size, dev, mnt);
    volume_t *vol = mnt->volume;
    fat_priv_data *sb = (fat_priv_data *)vol->sb;
    
    if (!buffer)
    {
        return 0;
    }
    fat_inode_t *fat_ino = (fat_inode_t*)node->inode;
    cluster cluster_start = fat_ino->first_cluster;
    cluster offset_cluster = offset / sb->bytes_per_cluster;
    if (offset_cluster != 0)
    {
        for (size_t i = 0; i < offset_cluster; i++)
        {
            cluster_start = fat_next_cluster(cluster_start, dev, sb);
            if (cluster_start >= FAT_CACHE_INVALID)
            {
                return 0;
            }
        }
    }

    uint32_t bytes_read = 0;
    uint32_t intra = offset % sb->bytes_per_cluster;
    log_debug(MODULE, "cluster_start = %u", cluster_start);
    while (size > 0 && cluster_start < FAT_CACHE_INVALID)
    {
        uint8_t tmp[sb->bytes_per_cluster];
        fat_read_clusters(tmp, cluster_start, 1, dev, sb);

        uint32_t available = sb->bytes_per_cluster - intra;
        uint32_t to_copy = size < available ? size : available;
        log_debug(MODULE, "memcpy(%p, %p, %u)", buffer + bytes_read, tmp + intra, to_copy);
        memcpy(buffer + bytes_read, tmp + intra, to_copy);

        bytes_read += to_copy;
        size -= to_copy;
        intra = 0;

        cluster_start = fat_next_cluster(cluster_start, dev, sb);
    }
    node->offset += bytes_read;
    return bytes_read;
}

int fat_entries_match(FAT_entry *entry, FAT_directory *directory, int entry_index, const char *name)
{
    if (entry->entry.attributes & FAT_ATTRIBUTE_LFN)
    {
        // TODO
    }

    char entry_name[13] = {0};
    if (fat_83_to_name(entry->entry.name, entry_name) != RETURN_GOOD)
    {
        log_err(MODULE, "entry name %s is not the right format", entry->entry.name);
        return RETURN_FAILED;
    }

    if (strcasecmp(name, entry_name) == 0)
    {
        return RETURN_GOOD;
    }
    return RETURN_FAILED;
}

int fat_read_directory(fat_inode_t *directory, FAT_directory *out, int entry_count, device_t *dev, mountpoint_t *mnt, fat_priv_data *sb)
{
    log_debug(MODULE, "fat_read_directory(%p, %p, %u, %p, %p, %p)", directory, out, entry_count, dev, mnt, sb);
    if (directory->base.type != DT_DIR)
    {
        return RETURN_ERROR;
    }
    uint32_64 byte_count = entry_count * sizeof(FAT_directory_entry);
    if (directory->first_cluster != sb->root_cluster)
    {
        // it is not the root dir
        if (byte_count > directory->base.size)
        {
            log_warn(MODULE, "reading more then directory size");
        }
    }
    cluster cluster_count = byte_count / sb->bytes_per_cluster;
    int bytes_read = fat_read_clusters(out->entries, directory->first_cluster, cluster_count, dev, sb);
    log_debug(MODULE, "read %i bytes", bytes_read);
    int entry_count_read = bytes_read / sizeof(FAT_directory_entry);
    return entry_count_read;
}

int fat_find_entry(fat_inode_t *parent, const char *entry_name, inode_t *entry_out, device_t *dev, mountpoint_t *mnt, fat_priv_data *sb)
{
    if (parent->base.type != DT_DIR)
    {
        log_warn(MODULE, "parent is not a directory");
        return RETURN_FAILED;
    }
    fat_inode_t *fat_ino = (fat_inode_t *)entry_out;

    size_t entries_in_cluster = (size_t)sb->bytes_per_cluster;

    FAT_entry *entries = malloc(entries_in_cluster);
    FAT_directory directory;
    directory.entries = entries;
    int entries_read = fat_read_directory(parent, &directory, entries_in_cluster / sizeof(FAT_directory_entry), dev, mnt, sb);
    if (entries_read <= 0)
    {
        free(entries);
        return RETURN_FAILED;
    }
    log_debug(MODULE, "printing %u entries", entries_read);
    size_t entry_name_length = strlen(entry_name);
    for (size_t i = 0; i < entries_read; i++)
    {
        FAT_directory_entry *entry = &(entries[i].entry);
        if (fat_entries_match(&entries[i], &directory, i, entry_name) == RETURN_GOOD)
        {
            log_info(MODULE, "entry %u @ %p {%s, %x, %u, %u}", i, entry, entry->name, entry->attributes, COMBINE_CLUSTERS((*entry)), entry->size);
            fat_ino->first_cluster = COMBINE_CLUSTERS((*entry));
            fat_ino->base.size = entry->size;
            if (entry->attributes & FAT_ATTRIBUTE_DIRECTORY)
            {
                fat_ino->base.type = DT_DIR;
            }
            free(entries);
            return RETURN_GOOD;
        }
    }
    free(entries);
    return RETURN_FAILED;
}

// fat driver
int fat_lookup(inode_t *parent, const char *name, inode_t *out, device_t *dev, mountpoint_t *mnt)
{
    log_debug(MODULE, "fat_lookup(%p, %s(%p), %p, %p, %p)", parent, name, name, out, dev, mnt);
    fat_priv_data *sb = (fat_priv_data *)mnt->volume->sb;

    fat_inode_t *fat_ino = (fat_inode_t *)out;
    if (parent == NULL && strcmp(name, "/") == 0)
    {
        // root inode request
        fat_ino->first_cluster = sb->root_cluster;
        out->ino = 0;
        out->type = DT_DIR;
        out->size = 0;
        return RETURN_GOOD;
    }

    fat_inode_t *fat_parent = (fat_inode_t *)parent;
    if (fat_find_entry(fat_parent, name, out, dev, mnt, sb) != RETURN_GOOD)
    {
        log_err(MODULE, "something is fucked");
    }
    return RETURN_GOOD;
}

int fat_open(vfs_node_t *node, device_t *dev, mountpoint_t *mnt)
{
    if (!node)
    {
        return RETURN_FAILED;
    }
    
    fat_inode_t *fat_ino = (fat_inode_t*)node->inode;

    // get the cluster number and size
    // fat_read_file(node, NULL, 0, 0, dev, mnt);
    return RETURN_GOOD;
}
int fat_close(vfs_node_t *node, device_t *dev, mountpoint_t *mnt)
{
    if (!node)
    {
        return RETURN_FAILED;
    }

    node->opened = false;
    node->offset = 0;
    node->inode = 0;
    node->size = 0;
    return RETURN_GOOD;
}

inode_t *fat_alloc_inode(volume_t *vol)
{
    fat_inode_t *ino = malloc(sizeof(fat_inode_t));
    memset(ino, 0, sizeof(fat_inode_t));
    ino->base.volume = vol;
    ino->base.fs = vol->fs;
    return &ino->base; // VFS only sees the base
}

void fat_free_inode(inode_t *ino)
{
    fat_inode_t *fat_ino = (fat_inode_t *)ino;
    free(fat_ino->cluster_chain);
    free(fat_ino);
}

int fat_mount(device_t *dev, mountpoint_t *mnt)
{
    volume_t *vol = mnt->volume;
    fat_priv_data *fat_priv = malloc(sizeof(fat_priv_data));
    memset(fat_priv, 0, sizeof(fat_priv_data));
    vol->sb = fat_priv;
    fat_priv->fat_cache_position = FAT_CACHE_INVALID;

    uint8_t buffer[512];
    FAT_boot_sector *bpb = (FAT_boot_sector *)buffer;
    dev->read(buffer, 0, 1, dev);

    fat_priv->bytes_per_sector = bpb->bytes_per_sector;
    fat_priv->sectors_per_cluster = bpb->sectors_per_cluster;
    fat_priv->reserved_sectors = bpb->reserved_sectors;
    fat_priv->fat_count = bpb->fat_count;
    uint16_t root_entry_count = bpb->directory_entry_count;

    fat_priv->sectors_per_fat = bpb->sectors_per_fat;
    if (fat_priv->sectors_per_fat == 0)
    {
        fat_priv->sectors_per_fat = bpb->EBR32.sectors_per_fat;
    }

    uint32_t root_sectors = (root_entry_count * 32 + fat_priv->bytes_per_sector - 1) / fat_priv->bytes_per_sector;

    uint32_t fat_start = fat_priv->reserved_sectors;
    uint32_t data_start = fat_start + fat_priv->fat_count * fat_priv->sectors_per_fat + root_sectors;
    fat_priv->fat_start_lba = fat_start;
    fat_priv->bytes_per_cluster = fat_priv->sectors_per_cluster * fat_priv->bytes_per_sector;

    uint32_t total_sectors = bpb->total_sectors;
    if (total_sectors == 0)
    {
        total_sectors = bpb->large_sector_count;
    }

    uint32_t cluster_count = (total_sectors - data_start) / fat_priv->sectors_per_cluster;

    if (cluster_count < 4085)
    {
        fat_priv->type = FAT12;
    }
    else if (cluster_count < 65525)
    {
        fat_priv->type = FAT16;
    }
    else
    {
        fat_priv->type = FAT32;
    }

    if (fat_priv->type == FAT32)
    {
        // FAT 32
        fat_priv->data_start_lba = data_start;
        fat_priv->root_cluster = bpb->EBR32.root_directory_cluster;
        fat_priv->root_start_lba = cluster_to_lba(fat_priv->root_cluster, fat_priv);
        fat_priv->root_sector_count = 0;
    }
    else
    {
        // FAT 12/16
        fat_priv->root_cluster = 0;
        fat_priv->root_start_lba = fat_start + fat_priv->fat_count * fat_priv->sectors_per_fat;
        fat_priv->root_sector_count = root_sectors;
        fat_priv->data_start_lba = fat_priv->root_start_lba + root_sectors;
    }

    return RETURN_GOOD;
}

int fat_probe(device_t *dev)
{
    if (!dev->read && dev->type == DEVICE_DISK)
    {
        return RETURN_FAILED;
    }

    uint8_t buffer[512];
    FAT_boot_sector *bpb = (FAT_boot_sector *)buffer;
    dev->read(buffer, 0, 1, dev);

    uint16_t sig = *(uint16_t *)(buffer + 510);
    if (sig != 0xAA55)
    {
        return RETURN_FAILED;
    }

    // bytes per sector must be 512, 1024, 2048, or 4096
    uint16_t bps = bpb->bytes_per_sector;
    if (bps != 512 && bps != 1024 && bps != 2048 && bps != 4096)
    {
        return RETURN_FAILED;
    }

    // sectors per cluster must be a power of 2
    uint8_t spc = bpb->sectors_per_cluster;
    if (spc == 0 || (spc & (spc - 1)) != 0)
    {
        return RETURN_FAILED;
    }

    return RETURN_GOOD;
}

static filesystem_t fat_driver;

void fat_init()
{
    fat_driver.name = "FAT";
    fat_driver.probe = fat_probe;
    fat_driver.mount = fat_mount;
    fat_driver.alloc_inode = fat_alloc_inode;
    fat_driver.free_inode = fat_free_inode;
    fat_driver.lookup = fat_lookup;
    fat_driver.open = fat_open;
    fat_driver.close = fat_close;
    fat_driver.read = fat_read_file;
    vfs_register_fs(&fat_driver);
}