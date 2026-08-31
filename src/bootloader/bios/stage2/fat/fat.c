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

#include "stdlib.h"
#include "memory.h"
#include "string.h"
#include "minmax.h"

#include "debug/debug.h"
#include "stdio.h"

#include <config.h>

#define MAX_PATH_SIZE         CONFIG_MAX_PATH_LENGTH

#define SECTOR_SIZE           512
#define MAX_FILE_HANDLES      10
#define ROOT_DIRECTORY_HANDLE -1
#define FAT_CACHE_SIZE        5

#define FAT12_EOC             0xFF8
#define FAT16_EOC             0xFFF8
#define FAT32_EOC             0x0FFFFFF8
#define FAT_CACHE_INVALID     0xFFFFFFFF

typedef struct
{
    // extended boot record
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;       // serial number, value doesn't matter
    uint8_t volume_label[11]; // 11 bytes, padded with spaces
    uint8_t system_id[8];
} PACKED fat_extended_boot_record_t;

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

} PACKED fat32_extended_boot_record_t;

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

} PACKED fat_boot_sector_t;

typedef struct
{
    uint8_t buffer[SECTOR_SIZE * 2];
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

    uint32_t data_section_lba;
    uint8_t fat_type;
    uint32_t total_sectors;
    uint32_t sectors_per_fat;
    uint32_t bytes_per_cluster;
} fat_data_t;

extern char __fat_driver;

int fat_read_sectors(partition_t *part, unsigned long lba, int count, void *buffer)
{
    // ENTER_FUNC("0x%p, %u, %u, 0x%p", part, lba, count, buffer);
    return partition_read(part, lba, count, buffer);
}

int fat_read_fat(partition_t *part, size_t lbaIndex, fat_data_t *fat_data)
{
    // ENTER_FUNC("0x%p, %u, 0x%p", part, lbaIndex, fat_data);
    return fat_read_sectors(part, fat_data->boot_sector.reserved_sectors + lbaIndex, FAT_CACHE_SIZE, fat_data->fat_cache);
}

uint32_t fat_cluster_to_lba(partition_t *part, fat_data_t *fat_data, uint32_t cluster)
{
    // ENTER_FUNC("0x%p, 0x%p, %u", part, fat_data, cluster);
    return fat_data->data_section_lba + (cluster - 2) * fat_data->boot_sector.sectors_per_cluster;
}

int fat_compare_lfn_blocks(const void *blockA, const void *blockB)
{
    fat_lfn_block_t *a = (fat_lfn_block_t *)blockA;
    fat_lfn_block_t *b = (fat_lfn_block_t *)blockB;
    return ((int)a->order) - ((int)b->order);
}

void fat_detect(partition_t *part, fat_data_t *fat_data)
{
    uint32_t dataClusters = (fat_data->total_sectors - fat_data->data_section_lba) / fat_data->boot_sector.sectors_per_cluster;
    if (dataClusters < 0xFF5)
    {
        fat_data->fat_type = 12;
    }
    else if (fat_data->boot_sector.sectors_per_fat != 0)
    {
        fat_data->fat_type = 16;
    }
    else
    {
        fat_data->fat_type = 32;
    }
}

int fat_probe(partition_t *part, fat_data_t *fat_data)
{
    printf("jump = 0x%x\n", fat_data->boot_sector.boot_jump_instruction[0]);
    if (fat_data->boot_sector.boot_jump_instruction[0] != 0xEB && fat_data->boot_sector.boot_jump_instruction[0] != 0xE9)
    {
        return 1;
    }
    return 0;
}

extern void hexdump(void *ptr, int len);

int fat_initialize(partition_t *part)
{
    fat_data_t *fat_data = (void *)&__fat_driver;
    if (fat_read_sectors(part, 0, 1, fat_data->boot_sector_bytes) != 0)
    {
        printf("FAT: read boot sector failed\r\n");
        return false;
    }

    if (fat_probe(part, fat_data))
    {
        printf("FAT: part doesn't use FAT\n");
        return 1;
    }

    // read FAT
    fat_data->fat_cache_position = 0xFFFFFFFF;

    fat_data->total_sectors = fat_data->boot_sector.total_sectors;
    if (fat_data->total_sectors == 0)
    { // fat32
        fat_data->total_sectors = fat_data->boot_sector.large_sector_count;
    }

    bool is_fat_32 = false;
    fat_data->sectors_per_fat = fat_data->boot_sector.sectors_per_fat;
    if (fat_data->sectors_per_fat == 0)
    { // fat32
        printf("is fat32\n");
        is_fat_32 = true;
        fat_data->sectors_per_fat = fat_data->boot_sector.ebr32.sectors_per_fat;
    }
    else
    {
        printf("is fat12/16\n");
    }

    // open root directory file
    uint32_t root_dir_lba;
    uint32_t root_dir_size;
    if (is_fat_32)
    {
        printf("res %u, spf %u, fc %u\n", fat_data->boot_sector.reserved_sectors, fat_data->sectors_per_fat, fat_data->boot_sector.fat_count);
        fat_data->data_section_lba = fat_data->boot_sector.reserved_sectors + fat_data->sectors_per_fat * fat_data->boot_sector.fat_count;
        root_dir_lba = fat_cluster_to_lba(part, fat_data, fat_data->boot_sector.ebr32.root_directory_cluster);
        root_dir_size = 0;
    }
    else
    {
        root_dir_lba = fat_data->boot_sector.reserved_sectors + fat_data->sectors_per_fat * fat_data->boot_sector.fat_count;
        root_dir_size = sizeof(fat_directory_entry_t) * fat_data->boot_sector.dir_entry_count;
        uint32_t root_dir_sectors = (root_dir_size + fat_data->boot_sector.bytes_per_sector - 1) / fat_data->boot_sector.bytes_per_sector;
        fat_data->data_section_lba = root_dir_lba + root_dir_sectors;
    }
    fat_data->bytes_per_cluster = fat_data->boot_sector.sectors_per_cluster * fat_data->boot_sector.bytes_per_sector;

    printf("DataSector: %u RootSector %u\r\n", fat_data->data_section_lba, root_dir_lba);

    fat_data->root_directory.public.handle = ROOT_DIRECTORY_HANDLE;
    fat_data->root_directory.public.is_directory = true;
    fat_data->root_directory.public.position = 0;
    fat_data->root_directory.public.size = sizeof(fat_directory_entry_t) * fat_data->boot_sector.dir_entry_count;
    fat_data->root_directory.opened = true;
    fat_data->root_directory.first_cluster = root_dir_lba;
    fat_data->root_directory.current_cluster = root_dir_lba;
    fat_data->root_directory.current_sector_in_cluster = 0;

    if (fat_read_sectors(part, root_dir_lba, 1, fat_data->root_directory.buffer) != 0)
    {
        hexdump(fat_data->root_directory.buffer, fat_data->boot_sector.bytes_per_sector);
        printf("FAT: read root directory failed\r\n");
        return false;
    }

    // calculate data section
    fat_detect(part, fat_data);

    // reset opened files
    for (int i = 0; i < MAX_FILE_HANDLES; i++)
    {
        memset(&fat_data->opened_files[i], 0, sizeof(fat_file_data_t));
        memset(&fat_data->opened_files[i].public, 0, sizeof(fat_file_t));
    }
    fat_data->lfn_count = 0;

    part->priv = fat_data;

    return 1;
}

int fat_read_entry(partition_t *part, fat_file_t *file, fat_directory_entry_t *dir_entry)
{
    // ENTER_FUNC("0x%p, 0x%p, 0x%p", part, file, dir_entry);
    return fat_read(part, file, sizeof(fat_directory_entry_t), dir_entry);
}

extern int fat_83_to_name(const char raw83[11], char out[13]);
extern int fat_name_to_83(const char *name, char out83[11]);

INTERNAL int fat_find_file(partition_t *part, fat_file_t *file, const char *name, fat_directory_entry_t *entry_out, fat_data_t *fat_data)
{
    // ENTER_FUNC("0x%p, 0x%p, '%s', 0x%p, 0x%p", part, file, name, entry_out, fat_data);
    char longName[256];
    fat_directory_entry_t entry;

    while (fat_read_entry(part, file, &entry))
    {
        if (entry.attributes == FAT_ATTRIBUTE_LFN)
        {
            fat_long_file_entry_t *lfn_entry = (fat_long_file_entry_t *)&entry;
            // printf("entry @ 0x%p {0x%x, 0x%x, 0x%x, 0x%x}\r\n", lfn_entry, lfn_entry->order, entry.attributes, lfn_entry->checksum, lfn_entry->long_entry_type);

            int idx = fat_data->lfn_count++;
            fat_data->lfn_blocks[idx].order = lfn_entry->order & (FAT_LFN_LAST - 1);
            memcpy(fat_data->lfn_blocks[idx].chars, lfn_entry->chars1, sizeof(lfn_entry->chars1));
            memcpy(fat_data->lfn_blocks[idx].chars + 5, lfn_entry->chars2, sizeof(lfn_entry->chars2));
            memcpy(fat_data->lfn_blocks[idx].chars + 11, lfn_entry->chars3, sizeof(lfn_entry->chars3));

            // is this the last LFN block
            if (lfn_entry->order == 1)
            {
                qsort(fat_data->lfn_blocks, fat_data->lfn_count, sizeof(fat_lfn_block_t), fat_compare_lfn_blocks);
                char *namePos = longName;
                for (int i = 0; i < fat_data->lfn_count; i++)
                {
                    int16_t *chars = fat_data->lfn_blocks[i].chars;
                    int16_t *charsLimit = chars + 13;

                    while (chars < charsLimit && *chars != 0)
                    {
                        int codepoint;
                        chars = utf16_to_codepoint(chars, &codepoint);
                        namePos = codepoint_to_utf8(codepoint, namePos);
                    }
                }
                *namePos = 0;
                // printf("LFN: %s\n", longName);
                // printf("entry @ 0x%p {%s, 0x%x, %u, %u}\r\n", &entry, longName, entry.attributes, ((entry).first_cluster_high << 16) | (entry).first_cluster_low, entry.size);
                // printf("entry = %s == %s\n", longName, name);
                fat_data->lfn_count = 0;
                if (strcmp(longName, name) == 0)
                {
                    fat_read_entry(part, file, &entry);
                    *entry_out = entry;
                    return 0;
                }
            }
        }
        else
        {
            // printf("entry @ 0x%p {%s, 0x%x, %u, %u}\r\n", &entry, entry.name, entry.attributes, ((entry).first_cluster_high << 16) | (entry).first_cluster_low, entry.size);
            if (fat_name_to_83(name, longName) != 0)
            {
                printf("Error failed to convert %s", entry.name);
                return 1;
            }
            // printf("entry = '%s' == '%s'\n", longName, entry.name);
            if (memcmp(longName, (const char *)entry.name, 11) == 0)
            {
                // printf("yes entry = '%s' == '%s'\n", longName, entry.name);
                *entry_out = entry;
                return 0;
            }
        }
    }
    return 1;
}

bool fat_exist(partition_t *part, const char *path)
{
    // ENTER_FUNC("0x%p, '%s'", part, path);
    fat_file_t *file = fat_open(part, path);
    if (file == NULL)
    {
        return false;
    }

    fat_close(part, file);
    return true;
}

fat_file_t *fat_open_entry(partition_t *part, fat_directory_entry_t *entry, fat_data_t *fat_data)
{
    // ENTER_FUNC("0x%p, 0x%p, 0x%p", part, entry, fat_data);
    // find empty handle
    int handle = MAX_FILE_HANDLES + 1;
    for (int i = 0; i < MAX_FILE_HANDLES; i++)
    {
        printf("FAT: handle %u is %u\r\n", i, fat_data->opened_files[i].opened);
        if (!fat_data->opened_files[i].opened)
        {
            handle = i;
            break;
        }
    }

    // out of handles
    if (handle == MAX_FILE_HANDLES + 1)
    {
        printf("FAT: out of file handles\r\n");
        return false;
    }
    printf("FAT: opening %s with handle %u\r\n", entry->name, handle);
    fat_data->opened_files[handle].opened = true;

    // setup vars
    fat_file_data_t *fd = &fat_data->opened_files[handle];
    fd->public.handle = handle;
    fd->public.is_directory = (entry->attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->public.position = 0;
    fd->public.size = entry->size;
    fd->first_cluster = (entry->first_cluster_high << 16) | entry->first_cluster_low;
    fd->current_cluster = fd->first_cluster;
    fd->current_sector_in_cluster = 0;

    if (fat_read_sectors(part, fat_cluster_to_lba(part, fat_data, fd->current_cluster), 1, fd->buffer) != 0)
    {
        printf("entry @ 0x%p {%s, 0x%x, %u, %u}\r\n", entry, entry->name, entry->attributes, ((entry)->first_cluster_high << 16) | (entry)->first_cluster_low, entry->size);
        printf("FAT: open entry failed - read error cluster=%u lba=%u\n", fd->current_cluster, fat_cluster_to_lba(part, fat_data, fd->current_cluster));
        for (int i = 0; i < 11; i++)
        {
            printf("%c", entry->name[i]);
        }
        printf("\n");
        return false;
    }

    fd->opened = true;
    return &fd->public;
}

fat_file_t *fat_open(partition_t *part, const char *path)
{
    // ENTER_FUNC("0x%p, '%s'", part, path);
    fat_data_t *fat_data = (fat_data_t *)part->priv;
    char name[MAX_PATH_SIZE];

    // ignore leading slash
    if (path[0] == '/')
    {
        path++;
    }

    fat_file_t *current = &fat_data->root_directory.public;
    fat_data->root_directory.current_cluster = fat_data->root_directory.first_cluster;
    fat_data->root_directory.public.position = 0;

    while (*path)
    {
        // extract next file name from path
        bool isLast = false;
        const char *delim = strchr(path, '/');
        if (delim != NULL)
        {
            memcpy(name, path, delim - path);
            name[delim - path] = '\0';
            path = delim + 1;
        }
        else
        {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len] = '\0';
            path += len;
            isLast = true;
        }

        // find directory entry in current directory
        fat_directory_entry_t entry;
        if (fat_find_file(part, current, name, &entry, fat_data) == 0)
        {
            fat_close(part, current);

            // check if directory
            if (!isLast && (entry.attributes & FAT_ATTRIBUTE_DIRECTORY) == 0)
            {
                printf("FAT: %s not a directory\r\n", name);
                return NULL;
            }

            // open new directory entry
            current = fat_open_entry(part, &entry, fat_data);
        }
        else
        {
            fat_close(part, current);

            printf("FAT: '%s' not found\r\n", name);
            return NULL;
        }
    }

    return current;
}

bool fat_is_eoc(uint32_t cluster, fat_data_t *fat_data)
{
    switch (fat_data->fat_type)
    {
        case 12 :
            return cluster >= FAT12_EOC;
        case 16 :
            return cluster >= FAT16_EOC;
        case 32 :
            return cluster >= FAT32_EOC;
    }
    return true;
}

uint32_t fat_next_cluster(partition_t *part, uint32_t current_cluster, fat_data_t *fat_data)
{
    // Determine the byte offset of the entry we need to read
    uint32_t fat_index;
    if (fat_data->fat_type == 12)
    {
        fat_index = current_cluster * 3 / 2;
    }
    else if (fat_data->fat_type == 16)
    {
        fat_index = current_cluster * 2;
    }
    else
    {
        fat_index = current_cluster * 4;
    }

    // Make sure cache has the right number
    uint32_t fat_index_sector = fat_index / fat_data->boot_sector.bytes_per_sector;
    if (fat_index_sector < fat_data->fat_cache_position || fat_index_sector >= fat_data->fat_cache_position + FAT_CACHE_SIZE)
    {
        fat_read_sectors(part, fat_data->boot_sector.reserved_sectors + fat_index_sector, FAT_CACHE_SIZE, fat_data->fat_cache);
        fat_data->fat_cache_position = fat_index_sector;
    }

    fat_index -= fat_data->fat_cache_position * fat_data->boot_sector.bytes_per_sector;

    uint32_t next_cluster;
    if (fat_data->fat_type == 12)
    {
        if (current_cluster % 2 == 0)
        {
            next_cluster = (*(uint16_t *)(fat_data->fat_cache + fat_index)) & 0x0FFF;
        }
        else
        {
            next_cluster = (*(uint16_t *)(fat_data->fat_cache + fat_index)) >> 4;
        }
    }
    else if (fat_data->fat_type == 16)
    {
        next_cluster = *(uint16_t *)(fat_data->fat_cache + fat_index);
    }
    else
    {
        next_cluster = *(uint32_t *)(fat_data->fat_cache + fat_index);
    }

    if (fat_is_eoc(next_cluster, fat_data))
    {
        return FAT_CACHE_INVALID;
    }

    return next_cluster;
}

int fat_read(partition_t *part, fat_file_t *file, uint32_t byte_count, void *buffer)
{
    // ENTER_FUNC("0x%p, 0x%p, %u, 0x%p", part, file, byte_count, buffer);
    if (!buffer)
    {
        return 1;
    }
    fat_data_t *fat_data = (fat_data_t *)part->priv;

    // get file data
    fat_file_data_t *fd = (file->handle == ROOT_DIRECTORY_HANDLE)
        ? &fat_data->root_directory
        : &fat_data->opened_files[file->handle];

    uint32_t offset = fd->public.position;
    uint32_t cluster_start = fd->first_cluster;
    uint32_t offset_cluster = offset / fat_data->bytes_per_cluster;
    if (offset_cluster != 0)
    {
        printf("offset_cluster = %u\n", offset_cluster);
        for (size_t i = 0; i < offset_cluster; i++)
        {
            uint32_t next_cluster = fat_next_cluster(part, cluster_start, fat_data);
            printf("next_cluster = %u, cluster_start = %u\n", next_cluster, cluster_start);
            cluster_start = next_cluster;
            if (next_cluster >= FAT_CACHE_INVALID)
            {
                return 0;
            }
        }
    }

    uint8_t *u8DataOut = (uint8_t *)buffer;

    uint16_t count = 0;
    size_t bytes_read = 0;
    uint32_t intra = offset % fat_data->bytes_per_cluster;
    size_t size = byte_count;

    printf("fd->current_cluster = %u, cluster_start = %u\r\n", fd->current_cluster, cluster_start);
    if (fd->current_cluster != 0 && fat_cluster_to_lba(part, fat_data, cluster_start) == fat_cluster_to_lba(part, fat_data, fd->current_cluster) + fd->current_sector_in_cluster && size < sizeof(fd->buffer))
    {
        uint32_t leftInBuffer = sizeof(fd->buffer) - (fd->public.position % sizeof(fd->buffer));
        uint32_t take = min(byte_count, leftInBuffer);
        printf("%u: leftInBuffer=%lu take=%lu\r\n", count, leftInBuffer, take);

        memcpy(buffer, fd->buffer + fd->public.position % sizeof(fd->buffer), take);
        printf("memcpy(%p, %p, %u)\r\n", u8DataOut, fd->buffer + fd->public.position % sizeof(fd->buffer), take);
        u8DataOut += take;
        fd->public.position += take;
        byte_count -= take;

        // hexdump(buffer, take);

        printf("%u: leftInBuffer=%lu take=%lu\r\n", count, leftInBuffer, take);
        return take;
    }
    else
    {
    }

    while (size > 0 && cluster_start < FAT_CACHE_INVALID)
    {
        uint8_t tmp[fat_data->bytes_per_cluster];
        if (fd->public.handle == ROOT_DIRECTORY_HANDLE)
        {
            fat_read_sectors(part, cluster_start, fat_data->boot_sector.sectors_per_cluster, tmp);
        }
        else
        {
            fat_read_sectors(part, fat_cluster_to_lba(part, fat_data, cluster_start), fat_data->boot_sector.sectors_per_cluster, tmp);
        }

        // hexdump(tmp, fat_data->bytes_per_cluster);

        uint32_t available = fat_data->bytes_per_cluster - intra;
        uint32_t to_copy = size < available ? size : available;
        printf("buffer = 0x%p\n", buffer);
        printf("bytes_read = 0x%x\n", bytes_read);
        printf("tmp = 0x%p\n", tmp);
        printf("intra = 0x%x\n", intra);
        printf("to_copy = 0x%x\n", to_copy);
        printf("memcpy(%p, %p, %u)\n", buffer + bytes_read, tmp + intra, to_copy);
        memcpy(buffer + bytes_read, tmp + intra, to_copy);

        bytes_read += to_copy;
        size -= to_copy;
        intra = 0;

        if (fd->public.handle != ROOT_DIRECTORY_HANDLE)
        {
            cluster_start = fat_next_cluster(part, cluster_start, fat_data);
        }
    }
    fd->public.position += bytes_read;
    if (fd->public.handle != ROOT_DIRECTORY_HANDLE)
    {
        // calculate next cluster & sector to read
        printf("FAT: fd->current_cluster = %u/0x%x\r\n", cluster_start, cluster_start);
        if (fd->current_sector_in_cluster++ >= fat_data->boot_sector.sectors_per_cluster)
        {
            fd->current_sector_in_cluster = 0;
            fd->current_cluster = cluster_start;
        }
        printf("FAT: fd->current_cluster = %u/0x%x\r\n", cluster_start, cluster_start);

        // read next sector
        if (fat_read_sectors(part, fat_cluster_to_lba(part, fat_data, cluster_start) + fd->current_sector_in_cluster, 2, fd->buffer) != 0)
        {
            printf("FAT: read error!\r\n");
            EXIT_FUNC(-2);
            return -2;
        }
        // hexdump(fd->buffer, sizeof(fd->buffer));
        fd->current_cluster = cluster_start;
    }
    return bytes_read;
    /* while (byte_count > 0)
    {
        uint32_t leftInBuffer = fat_data->bytes_per_cluster - (fd->public.position % fat_data->bytes_per_cluster);
        uint32_t take = min(byte_count, leftInBuffer);

        // memcpy(u8DataOut, fd->buffer + fd->public.position % fat_data->bytes_per_cluster, take);
        // printf("memcpy(%p, %p, %u)\r\n", u8DataOut, fd->buffer + fd->public.position % fat_data->bytes_per_cluster, take);
        // u8DataOut += take;
        // fd->public.position += take;
        // byte_count -= take;

        // printf("%u: leftInBuffer=%lu take=%lu\r\n", count, leftInBuffer, take);
        // count++;
        // See if we need to read more data
        // if (leftInBuffer == take)
        {
            // Special handling for root directory
            if (fd->public.handle == ROOT_DIRECTORY_HANDLE)
            {
                fd->current_cluster++;

                // read next sector
                if (fat_read_sectors(part, fd->current_cluster, fat_data->boot_sector.sectors_per_cluster, buffer) != 0)
                {
                    printf("FAT: read error!\r\n");
                    EXIT_FUNC(-1);
                    return -1;
                }
            }
            else
            {
                // calculate next cluster & sector to read
                printf("FAT: fd->current_cluster = %u/0x%x\r\n", fd->current_cluster, fd->current_cluster);
                if (++fd->current_cluster >= fat_data->boot_sector.sectors_per_cluster)
                {
                    fd->current_sector_in_cluster = 0;
                    fd->current_cluster = fat_next_cluster(part, fd->current_cluster, fat_data);
                }
                if (fd->current_cluster >= FAT_CACHE_INVALID)
                {
                    // Mark end of file
                    fd->public.size = fd->public.position + take;
                    break;
                }
                printf("FAT: fd->current_cluster = %u/0x%x\r\n", fd->current_cluster, fd->current_cluster);

                // read next sector
                if (fat_read_sectors(part, fat_cluster_to_lba(part, fat_data, fd->current_cluster) + fd->current_sector_in_cluster, fat_data->boot_sector.sectors_per_cluster, buffer) != 0)
                {
                    printf("FAT: read error!\r\n");
                    EXIT_FUNC(-2);
                    return -2;
                }
            }
        }
        fd->public.position += take;
        byte_count -= take;

        printf("%u: leftInBuffer=%lu take=%lu\r\n", count, leftInBuffer, take);
        count++;
    } */

    return u8DataOut - (uint8_t *)buffer;
}

void fat_close(partition_t *part, fat_file_t *file)
{
    // ENTER_FUNC("0x%p, 0x%p", part, file);
    fat_data_t *fat_data = (fat_data_t *)part->priv;
    if (file->handle == ROOT_DIRECTORY_HANDLE)
    {
        file->position = 0;
        fat_data->root_directory.current_cluster = fat_data->root_directory.first_cluster;
    }
    else
    {
        memset(&fat_data->opened_files[file->handle], 0, sizeof(fat_file_data_t));
        fat_data->opened_files[file->handle].opened = false;
    }
}

int fat_seek(partition_t *part, fat_file_t *file, uint32_t position)
{
    // ENTER_FUNC("0x%p, 0x%p, 0x%x", part, file, position);
    fat_data_t *fat_data = (fat_data_t *)part->priv;
    fat_file_data_t *fd = (file->handle == ROOT_DIRECTORY_HANDLE) ? &fat_data->root_directory : &fat_data->opened_files[file->handle];
    file->position = position;
    if (position / sizeof(fd->buffer) > 0)
    {
        fd->current_cluster = 0; // rewind
        fd->current_sector_in_cluster = 0;
    }
    // int code = fat_skip(part, file, position); // then skip forward
    printf("file->position = %u\n", file->position);
    printf("fd->current_cluster = %u\n", fd->current_cluster);
    return 0;
}
