/*
 * File: fat.c
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fat.h"
#include "vfs/fs_registry.h"
#include "vfs/volume.h"

#include "memory.h"
#include "string.h"

#include "kerrno.h"

#define MODULE "fat"

ALWAYS_INLINE sector_t bytes_to_lba(size_t bytes, fat_priv_data_t *sb)
{
    return bytes / sb->bytes_per_sector;
}

ALWAYS_INLINE size_t lba_to_bytes(sector_t lba, fat_priv_data_t *sb)
{
    return lba * sb->bytes_per_sector;
}

ALWAYS_INLINE cluster_t bytes_to_cluster(size_t bytes, fat_priv_data_t *sb)
{
    sector_t lba = bytes / sb->bytes_per_sector;
    return ((lba - sb->data_start_lba) / sb->sectors_per_cluster) + 2;
}

ALWAYS_INLINE size_t cluster_to_bytes(cluster_t clusters, fat_priv_data_t *sb)
{
    sector_t l = sb->data_start_lba + (clusters - 2) * sb->sectors_per_cluster;
    return l * sb->bytes_per_sector;
}

ALWAYS_INLINE sector_t cluster_to_lba(cluster_t cluster, fat_priv_data_t *sb)
{
    return sb->data_start_lba + (cluster - 2) * sb->sectors_per_cluster;
}

ALWAYS_INLINE cluster_t lba_to_cluster(sector_t lba, fat_priv_data_t *sb)
{
    return ((lba - sb->data_start_lba) / sb->sectors_per_cluster) + 2;
}

status_t fat_read_sectors(void *buffer, sector_t sector, size_t sector_count, device_t *dev, fat_priv_data_t *sb, ssize_t *out)
{
    // ENTER_FUNC(MODULE, "%p, %u, %u, %p, %p", buffer, sector, sector_count, dev, sb);
    ssize_t sector_read = 0;
    status_t ret = dev->ops.read(dev, buffer, sector_count, sector, &sector_read);
    *out = sector_read * sb->bytes_per_sector;
    return ret;
}

status_t fat_read_clusters(void *buffer, cluster_t cluster_start, cluster_t clusters_count, device_t *dev, fat_priv_data_t *sb, ssize_t *out)
{
    // ENTER_FUNC(MODULE, "%p, %u, %u, %p, %p", buffer, cluster_start, clusters_count, dev, sb);
    sector_t lba = cluster_to_lba(cluster_start, sb);
    return fat_read_sectors(buffer, lba, clusters_count * sb->sectors_per_cluster, dev, sb, out);
}

extern void hexdump(void *ptr, size_t len, size_t size);

status_t fat_mount(device_t *dev, mountpoint_t *mnt)
{
    volume_t *vol = mnt->volume;
    fat_priv_data_t *fat_priv = kzmalloc(sizeof(fat_priv_data_t));

    vol->sb = fat_priv;
    fat_priv->fat_cache_position = FAT_CACHE_INVALID;

    uint8_t buffer[512];
    fat_boot_sector_t *bpb = (fat_boot_sector_t *)buffer;
    ssize_t sector_read = 0;
    dev->ops.read(dev, buffer, 1, 0, &sector_read);

    fat_priv->bytes_per_sector = bpb->bytes_per_sector;
    fat_priv->sectors_per_cluster = bpb->sectors_per_cluster;
    fat_priv->reserved_sectors = bpb->reserved_sectors;
    fat_priv->fat_count = bpb->fat_count;
    uint16_t root_entry_count = bpb->directory_entry_count;

    fat_priv->sectors_per_fat = bpb->sectors_per_fat;
    if (fat_priv->sectors_per_fat == 0)
    {
        fat_priv->sectors_per_fat = bpb->ebr32.sectors_per_fat;
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
        fat_priv->root_cluster = bpb->ebr32.root_directory_cluster;
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

    return KERRNO_SUCCESSES;
}

status_t fat_probe(device_t *dev)
{
    if (!dev->ops.read && dev->class->type == DEVICE_BLOCK)
    {
        return KERRNO_UNSUCCESS;
    }

    uint8_t buffer[512];
    fat_boot_sector_t *bpb = (fat_boot_sector_t *)buffer;
    ssize_t sectors_read = 0;
    dev->ops.read(dev, buffer, 1, 0, &sectors_read);

    // hexdump(bpb, sizeof(fat_boot_sector_t), 16);

    uint16_t sig = *(uint16_t *)(buffer + 510);
    if (sig != 0xAA55)
    {
        return KERRNO_UNSUCCESS;
    }

    // bytes per sector must be 512, 1024, 2048, or 4096
    uint16_t bps = bpb->bytes_per_sector;
    if (bps != 512 && bps != 1024 && bps != 2048 && bps != 4096)
    {
        return KERRNO_UNSUCCESS;
    }

    // sectors per cluster must be a power of 2
    uint8_t spc = bpb->sectors_per_cluster;
    if (spc == 0 || (spc & (spc - 1)) != 0)
    {
        return KERRNO_UNSUCCESS;
    }

    return KERRNO_SUCCESSES;
}

status_t fat_ident_fs(device_t *dev, fs_ident_t *ident)
{
    if (!dev->ops.read && dev->class->type == DEVICE_BLOCK)
    {
        return KERRNO_UNSUCCESS;
    }

    uint8_t buffer[512];
    fat_boot_sector_t *bpb = (fat_boot_sector_t *)buffer;
    ssize_t sectors_read = 0;
    dev->ops.read(dev, buffer, 1, 0, &sectors_read);

    ident->uuid_len = 4;

    // sectors per cluster must be a power of 2
    uint8_t spf = bpb->sectors_per_fat;
    if (spf == 0)
    {
        // it is most likely fat32
        memcpy(ident->label, bpb->ebr32.ebr.volume_label, sizeof(bpb->ebr32.ebr.volume_label));
        memcpy(ident->uuid, &bpb->ebr32.ebr.volume_id, 4);
    }
    else
    {
        // it is most likely fat12 or 16
        memcpy(ident->label, bpb->ebr.volume_label, sizeof(bpb->ebr.volume_label));
        memcpy(ident->uuid, &bpb->ebr.volume_id, 4);
    }

    char *label = strchr(ident->label, ' ');
    if (label != NULL)
    {
        *label = '\0';
    }

    return KERRNO_SUCCESSES;
}

status_t fat_lookup(inode_t *parent, const char *name, inode_t *out, device_t *dev, mountpoint_t *mnt)
{
    fat_priv_data_t *sb = (fat_priv_data_t *)mnt->volume->sb;

    fat_inode_t *fat_ino = (fat_inode_t *)out;
    if (parent == NULL && strcmp(name, "/") == 0)
    {
        // root inode request
        fat_ino->first_cluster = sb->root_cluster;
        out->ino = 0;
        out->type = DT_DIR;
        out->size = 0;
        return KERRNO_SUCCESSES;
    }

    fat_inode_t *fat_parent = (fat_inode_t *)parent;
    status_t ret = fat_entry_find(fat_parent, name, out, dev, mnt, sb);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
        // log_err(MODULE, "something is fucked");
    }
    return KERRNO_SUCCESSES;
}

status_t fat_alloc_inode(volume_t *vol, inode_t **out)
{
    fat_inode_t *ino = kzmalloc(sizeof(fat_inode_t));
    ino->base.volume = vol;
    ino->base.fs = vol->fs;
    *out = &ino->base;
    return KERRNO_SUCCESSES;
}

static filesystem_t fat_fs = {
    .name = "FAT",
    .probe = fat_probe,
    .ident_fs = fat_ident_fs,
    .mount = fat_mount,
    .lookup = fat_lookup,
    .alloc_inode = fat_alloc_inode,
};

VFS_FS_DRIVER(fat_fs);