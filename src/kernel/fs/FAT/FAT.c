/*
 * File: FAT.c
 * File Created: 09 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "FAT.h"

#include "malloc.h"
#include "debug/debug.h"
#include "paging/paging.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#define MODULE "FAT"

#define PRINT_FUNCTION_INFO(func, ...) log_info(MODULE, "in function %s(%s)", func, __VA_ARGS__)

#define COMBINE_CLUSTERS(entry) (entry.FirstClusterHigh << 16) | entry.FirstClusterLow

uint32_t cluster_to_lba(uint32_t cluster, fat_priv_data *priv)
{
    return priv->data_start_lba + (cluster - 2) * priv->sectors_per_cluster;
}

uint32_t fat_read_sectors(void *buffer, uint32_t sector, uint32_t sector_count, device *dev, fat_priv_data *priv)
{
    return dev->read(buffer, sector, sector_count, dev);
}
/*
uint32_t fat_read_sectors(void *buffer, uint32_t bytes, uint32_t bytes_count, device *dev, fat_priv_data *priv)
{
    uint32_t sector = (bytes / priv->bytes_per_sector) + 1;
    uint32_t sector_count = (bytes_count / priv->bytes_per_sector) + 1;
    return dev->read(buffer, sector, sector_count, dev);
}
void fat_read_clusters(void *buffer, uint32_t *bytes, uint32_t bytes_count, device *dev, fat_priv_data *priv)
{

}
*/

bool fat_83_to_name(const char raw83[11], char out[13])
{
    if (!raw83 || !out)
        return false;

    // Deleted entry
    if (raw83[0] == 0xE5)
    {
        return false;
    }
    // Empty (end-of-directory) entry
    if (raw83[0] == 0x00)
    {
        return false;
    }

    char *p = out;

    // ---- Name part (bytes 0-7) ----
    for (int i = 0; i < 8; i++)
    {
        uint8_t c = raw83[i];
        // trailing spaces = end of name
        if (c == ' ')
        {
            break;
        }
        // Kanji / 0xE5 escape
        if (i == 0 && c == 0x05)
        {
            c = 0xE5;
        }
        *p++ = (char)c;
    }

    // ---- Extension part (bytes 8-10) ----
    // Check if there is a non-space extension
    int has_ext = 0;
    for (int i = 8; i < 11; i++)
    {
        if (raw83[i] != ' ')
        {
            has_ext = 1;
            break;
        }
    }

    if (has_ext)
    {
        *p++ = '.';
        for (int i = 8; i < 11; i++)
        {
            if (raw83[i] == ' ')
            {
                break;
            }
            *p++ = (char)raw83[i];
        }
    }

    *p = '\0';
    return 1;
}

bool fat_name_to_83(const char *name, uint8_t out83[11])
{
    if (!name || !out83 || name[0] == '\0')
    {
        return false;
    }

    // Pre-fill with spaces
    out83 = memset(out83, ' ', 11);

    // Find the LAST dot to split name / extension.
    // Special case: a leading dot (e.g. ".hidden") is part of the name, not
    // a name/ext separator.
    const char *dot = (const char *)0;
    for (const char *p = name + 1; *p; p++)
    { // start at +1 to skip leading dot
        if (*p == '.')
        {
            dot = p;
        }
    }

    const char *ext_start = dot ? dot + 1 : (const char *)0;
    int name_len = dot ? (int)(dot - name) : (int)strlen(name);

    // ---- Write name (bytes 0-7) ----
    int ni = 0;
    for (int i = 0; i < name_len && ni < 8; i++)
    {
        char c = toupper(name[i]);
        // skip spaces (illegal in FAT)
        if (c == ' ' || c == '\0')
        {
            continue;
        }
        out83[ni++] = (uint8_t)c;
    }

    // Kanji / deleted-entry escape
    if (out83[0] == 0xE5)
    {
        out83[0] = 0x05;
    }

    // ---- Write extension (bytes 8-10) ----
    if (ext_start)
    {
        int ei = 8;
        for (int i = 0; ext_start[i] != '\0' && ei < 11; i++)
        {
            char c = toupper(ext_start[i]);
            if (c == ' ')
            {
                continue;
            }
            out83[ei++] = (uint8_t)c;
        }
    }

    return true;
}

bool fat_is_eoc(fat_priv_data *priv, uint32_t cluster)
{
    switch (priv->type)
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

uint32_t fat_next_cluster(uint32_t current_cluster, device *dev, fat_priv_data *fat_priv)
{
    uint32_t fat_index;
    if (fat_priv->type == FAT12)
    {
        fat_index = current_cluster * 3 / 2;
    }
    else if (fat_priv->type == FAT16)
    {
        fat_index = current_cluster * 2;
    }
    else if (fat_priv->type == FAT32)
    {
        fat_index = current_cluster * 4;
    }

    uint32_t fat_index_sector = fat_index / fat_priv->bytes_per_sector;
    if (fat_index_sector < fat_priv->fat_cache_position || fat_index_sector >= fat_priv->fat_cache_position + FAT_CACHE_SIZE)
    {
        dev->read(fat_priv->fat_cache, fat_priv->fat_start_lba + fat_index_sector, FAT_CACHE_SIZE, dev);
        fat_priv->fat_cache_position = fat_index_sector;
    }

    fat_index -= fat_priv->fat_cache_position * fat_priv->bytes_per_sector;

    uint32_t next_cluster;

    if (fat_priv->type == FAT12)
    {
        if (current_cluster % 2 == 0)
        {
            next_cluster = (*(uint16_t *)(fat_priv->fat_cache + fat_index)) & 0x0FFF;
        }
        else
        {
            next_cluster = (*(uint16_t *)(fat_priv->fat_cache + fat_index)) >> 4;
        }
    }
    else if (fat_priv->type == FAT16)
    {
        next_cluster = *(uint16_t *)(fat_priv->fat_cache + fat_index);
    }
    else if (fat_priv->type == FAT32)
    {
        next_cluster = *(uint32_t *)(fat_priv->fat_cache + fat_index) & 0x0FFFFFFF;
    }

    if (fat_is_eoc(fat_priv, next_cluster))
    {
        return 0xFFFFFFFF;
    }

    return next_cluster;
}
/* 
void get_directory_entry_count(FAT_directory *directory, device *dev, fat_priv_data *fat_priv)
{

} */

void fat_get_root_directory(device *dev, fat_priv_data *fat_priv, FAT_directory *out)
{
    uint32_t root_directory_count = fat_priv->root_sector_count;
    if (fat_priv->type == FAT32)
    {
        uint32_t cluster_count = 0;
        uint32_t current = fat_priv->root_cluster;

        while (!fat_is_eoc(fat_priv, current))
        {
            cluster_count++;

            uint32_t next = fat_next_cluster(current, dev, fat_priv);
            if (next == 0)
            {
                break; // Error or end
            }
            current = next;
        }

        root_directory_count = cluster_count * fat_priv->sectors_per_cluster;
    }

    log_debug(MODULE, "reading from %u(%u), %u times", fat_priv->root_start_lba, fat_priv->root_cluster, root_directory_count);
    fat_read_sectors(out->entries, fat_priv->root_start_lba, root_directory_count, dev, fat_priv);
    uint32_t directory_size_bytes = fat_priv->root_start_lba * fat_priv->bytes_per_sector;
    out->entry_count = directory_size_bytes / 32;
}

bool fat_read_directory_internal(FAT_entry *entry, device *dev, fat_priv_data *fat_priv, FAT_directory *out)
{
    uint32_t directory_cluster = COMBINE_CLUSTERS(entry->entry);
    uint32_t directory_count = 0;

    {
        uint32_t cluster_count = 0;
        uint32_t current = fat_priv->root_cluster;

        while (!fat_is_eoc(fat_priv, current))
        {
            cluster_count++;

            uint32_t next = fat_next_cluster(current, dev, fat_priv);
            if (next == 0)
            {
                break; // Error or end
            }
            current = next;
        }

        directory_count = cluster_count * fat_priv->sectors_per_cluster;
    }

    uint32_t directory_sector = cluster_to_lba(directory_cluster, fat_priv);
    log_debug(MODULE, "reading from %u(%u), %u times", directory_sector, directory_cluster, directory_count);
    fat_read_sectors(out->entries, directory_sector, directory_count, dev, fat_priv);
    uint32_t directory_size_bytes = directory_sector * fat_priv->bytes_per_sector;
    out->entry_count = directory_size_bytes / 32;
    return true;
}

bool fat_find_entry(char *path, device *dev, fat_priv_data *fat_priv, FAT_directory *directory, FAT_entry **entry_out)
{
    char *org_path = path;
    if (*org_path != '/')
    {
        log_err(MODULE, "path '%s' wasn't rooted", path);
        PRINT_FUNCTION_INFO("fat_find_entry", "%s, %p, %p, %p", path, dev, fat_priv, entry_out);
        return false;
    }
    org_path++;

    // path segment in a human readable format
    char *path_segment_hr = malloc(MAX_FILE_NAME);
    memset(path_segment_hr, 0, MAX_FILE_NAME);
    {
        const char *temp = strchr(org_path, '/');
        int count = (int)(temp - org_path);
        memcpy(path_segment_hr, org_path, count);
        log_debug(MODULE, "path_segment = %s", path_segment_hr);
        org_path += count;
    }
    log_debug(MODULE, "in fat_find_entry directory = %p with org_path = %s (%u)", directory, org_path, *org_path);

    // path segment in the 8.3 format
    char segment83[12];
    memset(segment83, 0, 12);
    fat_name_to_83(path_segment_hr, (uint8_t *)segment83);
    free(path_segment_hr);
    log_debug(MODULE, "after 8.3 segment %s", segment83);

    directory->entry_count = 10;
    for (size_t i = 0; i < directory->entry_count; i++)
    {
        FAT_entry *entry = &(directory->entries[i]);
        log_debug(MODULE, "%u: entry at %p with name %s", i, entry, entry->entry.Name);

        if (memcmp(segment83, entry->entry.Name, 4) == 0)
        {
            log_debug(MODULE, "found entry at %p", entry);
            *entry_out = entry;
            break;
        }
    }

    if (*org_path == 0)
    {
        // done
        return true;
    }
    if (*(org_path - 1) == '/')
    {
        org_path--;
    }

    // read the entry
    FAT_directory *current_directory = malloc(sizeof(FAT_directory));
    if (!fat_read_directory_internal(*entry_out, dev, fat_priv, current_directory))
    {
        log_err(MODULE, "failed to read %s entry's directories", (*entry_out)->entry.Name);
        PRINT_FUNCTION_INFO("fat_find_entry", "%s, %p, %p, %p", path, dev, fat_priv, entry_out);
        return false;
    }

    bool result = fat_find_entry(org_path, dev, fat_priv, current_directory, entry_out);
    free(current_directory);
    return result;
}

bool fat_read_directory(vfs_node *dir, uint32_t index, device *dev, mount_point *mnt)
{
    FUNC_NOT_IMPLEMENTED(MODULE, "fat_read_directory");
    return false;
}

uint32_t fat_read_file(vfs_node *node, void *buffer, size_t offset, size_t length, device *dev, mount_point *mnt)
{
    /*
    log_debug(MODULE, "got node {");
    log_debug(MODULE, "  name = %s", node->name);
    log_debug(MODULE, "  flags = 0x%x", node->flags);
    log_debug(MODULE, "  size = %u", node->size);
    log_debug(MODULE, "  inode = %u", node->inode);
    log_debug(MODULE, "  offset = %u", node->offset);
    log_debug(MODULE, "  opened = %s", node->opened ToBoolString);
    log_debug(MODULE, "  fs = %p", node->fs);
    log_debug(MODULE, "  mount = %p", node->mount);
    log_debug(MODULE, "  priv = %p", node->priv);
    log_debug(MODULE, "}");
    */
    fat_priv_data *fat_priv = mnt->fs_priv;

    uint32_t cluster_number = node->inode;
    if (cluster_number == 0)
    {
        char *path = node->name;
        FAT_directory *current_directory = malloc(sizeof(FAT_directory));
        fat_get_root_directory(dev, fat_priv, current_directory);
        log_debug(MODULE, "post root current_directory = %p", current_directory);
        FAT_entry *entry = NULL;
        fat_find_entry(path, dev, fat_priv, current_directory, &entry);
        free(current_directory);
        log_debug(MODULE, "got entry at %p", entry);
        cluster_number = (entry->entry.FirstClusterHigh << 16) | entry->entry.FirstClusterLow;
        node->inode = cluster_number;
        node->size = entry->entry.Size;
    }
    /*
    log_debug(MODULE, "updated node {");
    log_debug(MODULE, "  name = %s", node->name);
    log_debug(MODULE, "  flags = 0x%x", node->flags);
    log_debug(MODULE, "  size = %u", node->size);
    log_debug(MODULE, "  inode = %u", node->inode);
    log_debug(MODULE, "  offset = %u", node->offset);
    log_debug(MODULE, "  opened = %s", node->opened ToBoolString);
    log_debug(MODULE, "  fs = %p", node->fs);
    log_debug(MODULE, "  mount = %p", node->mount);
    log_debug(MODULE, "  priv = %p", node->priv);
    log_debug(MODULE, "}");
    */
    if (!buffer)
    {
        return 0;
    }
    uint32_t lba = cluster_to_lba(cluster_number, fat_priv);
    uint32_t count = (length / fat_priv->bytes_per_sector) + 1;
    return fat_read_sectors(buffer, lba, count, dev, fat_priv);
}

bool fat_probe(device *dev)
{
    if (!dev->read)
    {
        return false;
    }

    uint8_t buffer[512];
    FAT_boot_sector *bpb = (FAT_boot_sector *)buffer;
    dev->read(buffer, 0, 1, dev);

    uint16_t sig = *(uint16_t *)(buffer + 510);
    if (sig != 0xAA55)
    {
        return false;
    }

    // bytes per sector must be 512, 1024, 2048, or 4096
    uint16_t bps = bpb->bytes_per_sector;
    if (bps != 512 && bps != 1024 && bps != 2048 && bps != 4096)
    {
        return false;
    }

    // sectors per cluster must be a power of 2
    uint8_t spc = bpb->sectors_per_cluster;
    if (spc == 0 || (spc & (spc - 1)) != 0)
    {
        return false;
    }

    return true;
}

bool fat_open(vfs_node *node, device *dev, mount_point *mnt)
{
    if (!node)
    {
        return false;
    }

    node->opened = true;
    node->offset = 0;
    // get the cluster number and size
    fat_read_file(node, NULL, 0, 0, dev, mnt);
    return true;
}
bool fat_close(vfs_node *node, device *dev, mount_point *mnt)
{
    if (!node)
    {
        return false;
    }

    node->opened = false;
    node->offset = 0;
    node->inode = 0;
    node->size = 0;
    return true;
}

bool fat_mount(device *dev, mount_point *mount)
{
    fat_priv_data *fat_priv = malloc(sizeof(fat_priv_data));
    mount->fs_priv = fat_priv;
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
    fat_priv->cluster_size_bytes = fat_priv->sectors_per_cluster * fat_priv->bytes_per_sector;

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

    return true;
}

filesystem *fat_init()
{
    filesystem *fs = (filesystem *)malloc(sizeof(filesystem));

    fs->name = "FAT";
    fs->mount = fat_mount;
    fs->probe = fat_probe;
    fs->open = fat_open;
    fs->close = fat_close;
    fs->read = fat_read_file;

    return fs;
}