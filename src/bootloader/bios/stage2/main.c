/*
 * File: main.c
 * File Created: 18 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

/// @file This is the first file in the stage 2 bootloader that is in C,
/// this is where the kernel is found and loaded using the FAT driver and ELF parser
/// this file's start function is called from entry.asm

#include "fat/fat.h"
#include "fs/disk.h"
#include "partition/partition.h"

#include "elf/elf.h"

#include "memdefs.h"
#include "memory.h"

#include "stdio.h"
#include "io/io.h"
#include "string.h"

#include "mem_detect.h"
#include "video_detect.h"
#include "acpi/acpi.h"
#include "kernel.h"

#include <defs.h>
#include <boot/params.h>
#include <stddef.h>
#include <stdint.h>
#include <config.h>

/// @brief The short name of the BareOS Bootloader which is placed into,
/// the bootparams bootloader_name field @see src/libs/boot/params.h
/// @author BjornBEs
#define BOOTLOADER_NAME "BARE_BOOT_1"

/// @brief The boot drive given from the BIOS
extern uint16_t boot_drive;

/// @brief The index in the MBR to the booting partition
extern uint16_t boot_partition_index;

/// @brief The pointer to the kernel's virtual address
/// @author BjornBEs
boot_start *kernel_entry;

/// @brief Will print out len hex bytes to stdout from the addresses in ptr
/// @param ptr The address the bytes will come from.
/// @param len The number of bytes to print
/// @author BjornBEs
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

/// @brief The start of the bootloader in C
/// @param partition_data Will point to the first entry in the MBR
/// @return This function will never return.
/// @pre The CPU needs to have set up the stack and enable a 32 bit mode before this or any C
/// functions are called
/// @author BjornBEs
NORETURN CDECL void start(void *partition_data)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    boot_params_t *boot_params = (boot_params_t *)MEMORY_BOOTPARAMS_ADDR;
    memset(boot_params, 0, sizeof(boot_params_t));
    boot_params->magic = BOOT_PARAMS_MAGIC;
    boot_params->struct_version = BOOT_PARAMS_VERSION;
    strcpy(boot_params->bootloader_name, BOOTLOADER_NAME);
    printf("boot_params @ 0x%x\n", boot_params);
#pragma GCC diagnostic pop
#ifdef __x86__
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    boot_params->arch.disabled_pic = true;
#endif

    printf("======== START ========\n");
    hexdump(partition_data, 64);

    printf("boot_drive = 0x%x\n", boot_drive);
    printf("boot_partition_index = %u\n", boot_partition_index);

    disk_t disk;
    int code = disk_initialize(&disk, boot_drive);
    if (code != 0)
    {
        printf("error: disk initialize failed with code %u\n", code);
        goto end;
    }

    partition_t part;
    code = partition_detect(&part, &disk, partition_data, boot_partition_index);
    if (code != 0)
    {
        hexdump((void *)(0x1BE + 0x7C00), 64);
        code = partition_detect(&part, &disk, (void *)(0x1BE + 0x7C00), boot_partition_index);
        if (code != 0)
        {
            printf("error: partition initialize failed with code %u\n", code);
            goto end;
        }
    }

    if (!fat_initialize(&part))
    {
        printf("FAT init error\r\n");
        goto end;
    }

    if (elf_read_kernel(&part, boot_params) == 1)
    {
        printf("The kernel fail to be read\n");
        goto end;
    }
    printf("ELF: kernel_entry_point = %llp\n", boot_params->kernel_entry_point);

    mm_arch_detect_memory(boot_params);

    mm_arch_detect_video(boot_params);

    acpi_detect(boot_params);

    printf("Got firmware info\n");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    kernel_entry = (boot_start *)boot_params->kernel_entry_point;
#pragma GCC diagnostic pop
    printf("kernel_entry = 0x%llp\n", kernel_entry);

    {
        enter_kernel(boot_params, kernel_entry);
        UNREACHABLE();
    }

end:
    printf("enter loop\n");
    for (;;)
    {
        ;
    }
}
