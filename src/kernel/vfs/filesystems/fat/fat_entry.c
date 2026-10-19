/*
 * File: fat_entry.c
 * File Created: 19 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fat.h"

#include "debug/debug.h"

#include "memory.h"
#include "string.h"

#include "kerrno.h"

#define MODULE "fat-entry"

int fat_entries_match(fat_entry_t *entry, fat_directory_t *directory, int entry_index, const char *name)
{
    if (entry->entry.attributes & FAT_ATTRIBUTE_LFN)
    {
        // TODO
    }

    uint32_t entry_name_size = strlen(entry->entry.name);
    if (entry_name_size == 0)
    {
        return KERRNO_UNSUCCESS;
    }
    char entry_name[13] = {0};
    if (fat_83_to_name(entry->entry.name, entry_name) != KERRNO_SUCCESSES)
    {
        trace_err(MODULE, "entry name %s is not the right format", entry->entry.name);
        return KERRNO_UNSUCCESS;
    }

    if (fat_name_eq((char*)name, entry_name) == KERRNO_SUCCESSES)
    {
        return KERRNO_SUCCESSES;
    }
    return KERRNO_UNSUCCESS;
}

int fat_read_directory(fat_inode_t *directory, fat_directory_t *out, int entry_count, device_t *dev, mountpoint_t *mnt, fat_priv_data_t *sb)
{
    if (directory->base.type != DT_DIR)
    {
        return KERRNO_UNSUCCESS;
    }
    size_t byte_count = entry_count * sizeof(fat_directory_entry_t);
    if (directory->first_cluster != sb->root_cluster)
    {
        // it is not the root dir
        if (byte_count > directory->base.size)
        {
            trace_warn(MODULE, "reading more then directory size");
            trace_debug(MODULE, "reading %u bytes instead of %u", byte_count, directory->base.size);
        }
    }
    cluster_t cluster_count = byte_count / sb->bytes_per_cluster;
    ssize_t bytes_read;
    status_t ret = fat_read_clusters(out->entries, directory->first_cluster, cluster_count, dev, sb, &bytes_read);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    // hexdump(out->entries, bytes_read);
    trace_debug(MODULE, "read %i bytes", bytes_read);
    int entry_count_read = bytes_read / sizeof(fat_directory_entry_t);
    return entry_count_read;
}

status_t fat_entry_find(fat_inode_t *parent, const char *entry_name, inode_t *entry_out, device_t *dev, mountpoint_t *mnt, fat_priv_data_t *sb)
{
    ENTER_FUNC(MODULE, "%p, %s, %p, %p, %p, %p", parent, entry_name, entry_out, dev, mnt, sb);
    if (parent->base.type != DT_DIR)
    {
        trace_warn(MODULE, "parent is not a directory");
        return KERRNO_UNSUCCESS;
    }
    fat_inode_t *fat_ino = (fat_inode_t *)entry_out;

    size_t entries_in_cluster = (size_t)sb->bytes_per_cluster;

    fat_entry_t *entries = kcalloc(entries_in_cluster / sizeof(fat_directory_entry_t), sizeof(fat_directory_entry_t));
    trace_debug(MODULE, "entries_in_cluster = %u", entries_in_cluster);
    fat_directory_t directory;
    directory.entries = entries;
    int entries_read = fat_read_directory(parent, &directory, entries_in_cluster / sizeof(fat_directory_entry_t), dev, mnt, sb);
    if (entries_read <= 0)
    {
        kfree(entries);
        return KERRNO_UNSUCCESS;
    }
    // hexdump(entries, entries_in_cluster);
    trace_debug(MODULE, "printing %u entries", entries_read);
    // size_t entry_name_length = strlen(entry_name);
    for (int i = 0; i < entries_read; i++)
    {
        fat_directory_entry_t *entry = &(entries[i].entry);
        trace_info(MODULE, "entry %u @ %p {%s, %x, %u, %u}", i, entry, entry->name, entry->attributes, COMBINE_CLUSTERS(*entry), entry->size);
        if (fat_entries_match(&entries[i], &directory, i, entry_name) == KERRNO_SUCCESSES)
        {
            trace_info(MODULE, "entry %u @ %p {%s, %x, %u, %u}", i, entry, entry->name, entry->attributes, COMBINE_CLUSTERS(*entry), entry->size);
            fat_ino->first_cluster = COMBINE_CLUSTERS(*entry);
            fat_ino->base.size = entry->size;
            fat_ino->created_time_tenths = entry->created_time_tenths;
            fat_ino->created_time.raw = entry->created_time;
            fat_ino->created_date.raw = entry->created_date;
            fat_ino->accessed_date.raw = entry->accessed_date;
            fat_ino->modified_time.raw = entry->modified_time;
            fat_ino->modified_date.raw = entry->modified_date;
            if (entry->attributes & FAT_ATTRIBUTE_DIRECTORY)
            {
                fat_ino->base.type = DT_DIR;
            }
            kfree(entries);
            return KERRNO_SUCCESSES;
        }
    }
    kfree(entries);
    return KERRNO_UNSUCCESS;
}
