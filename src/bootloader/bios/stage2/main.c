/*
 * File: main.c
 * File Created: 18 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "config.h"
#include "fat/fat.h"
#include "fs/disk.h"
#include "partition/partition.h"

#include "memdefs.h"

#include "stdio.h"
#include "io/io.h"

#include <boot/params.h>
#include <stddef.h>
#include <stdint.h>

extern uint8_t cpu_tier;
#ifdef __x86__
extern uint8_t have_cpuid;
#endif
extern uint16_t boot_drive;
extern uint16_t mbr_partition_segment;
extern uint16_t mbr_partition_offset;
extern uint16_t boot_partition_index;

void hexdump(void *ptr, int len)
{
    printf("========= HEXDUMP =========\n");
    printf("hexdump at 0x%p length %u\n", ptr, len);
    unsigned char *p = (unsigned char *)ptr;
    for (int i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
        {
            printf("\n%x: ", i);
        }
        printf("%x ", p[i]);
    }
    printf("\n");
}

void __attribute__((cdecl)) start(void *partition_data)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    boot_params_t *bootParams = (boot_params_t *)MEMORY_BOOTPARAMS_ADDR;
    printf("bootParams @ 0x%x\n", bootParams);
#pragma GCC diagnostic pop
    #ifdef __x86__
    outb(0x21,0xFF);
    outb(0xa1,0xFF);
    bootParams->arch.disabled_pic = true;
    #endif

    printf("======== START ========\n");
    hexdump(partition_data, 64);

    printf("cpu_tier = %u\n", cpu_tier);
    #ifdef __x86__
    printf("have_cpuid = %u\n", have_cpuid);
    #endif
    printf("boot_drive = 0x%x\n", boot_drive);
    printf("mbr_partition_segment = 0x%x\n", mbr_partition_segment);
    printf("mbr_partition_offset = 0x%x\n", mbr_partition_offset);
    printf("boot_partition_index = %u\n", boot_partition_index);

    disk_t disk;
    int code = disk_initialize(&disk, boot_drive);
    if (code != 0)
    {
        printf("error: disk initialize failed with code %u\n", code);
        goto end;
    }

    partition_t part;
    partition_detect(&part, &disk, partition_data, boot_partition_index);

    if (!fat_initialize(&part))
    {
        printf("FAT init error\r\n");
        goto end;
    }

    printf("disk: {id: %u, sectors: %u, cylinders: %u, heads: %u, extensions: %u}\n", disk.id, disk.sectors, disk.cylinders, disk.heads, disk.have_extensions);

end:
    printf("enter loop\n");
    for (;;)
    {
        ;
    }
}
