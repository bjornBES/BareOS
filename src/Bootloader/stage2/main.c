#include <stdint.h>
#include <stdbool.h>
#include "stdio.h"
#include <stddef.h>
#include "memory.h"
#include "string.h"

#include <boot/bootparams.h>
#include "memory/paging.h"
#include "x86.h"
#include "memdefs.h"
#include "fs/mbr.h"
#include "fs/disk.h"
#include "fs/fat.h"
#include "elf/elf.h"
#include "bootparams/memory/memdetect.h"
#include "bootparams/pci/pci.h"
#include "bootparams/video/vesa.h"
#include "menu/menu.h"

typedef void (*BootStart)(BootParams *boot);

BootStart kernelEntry;

void hexdump(void *ptr, int len)
{
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
            printf("\n%x: ", i);
        printf("%x ", p[i]);
    }
    printf("\n");
}

void __attribute__((cdecl)) start(uint16_t bootDrive, void *partition)
{
    bool readyToJump = false;
    printf("%x", partition);
    hexdump(partition, 64);

    BootParams *bootParams = (BootParams *)MEMORY_BOOTPARAMS_ADDR;

    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive))
    {
        printf("Disk init error\r\n");
        goto end;
    }

    Partition part;
    MBR_DetectPartition(&part, &disk, partition);

    if (!FAT_Initialize(&part))
    {
        printf("FAT init error\r\n");
        goto end;
    }

    bootParams->BootDevice = bootDrive;

    bootParams->bootLoader.bootFlags = 0;
    strcpy(bootParams->bootLoader.bootloaderName, "BESOS Bootloader");
    strcpy(bootParams->bootLoader.cmdline, "");

    DetectMemory(bootParams);
    DetectVESA(bootParams);
    DetectPCI(bootParams);

    /*
    Get some info of computer from BIOS like
    0x12              –                         Conventional memory
    0x15 (0x88, 0xE801, 0xE820, 0xC7)           Extended / full memory map, system config
    0x11              –                         Equipment list (floppy, video, etc.)
    0x13 (0x00, 0x02, 0x03, 0x08, 0x41)         Disk reset, read/write sectors, get drive params, LBA
    0x10 (0x0F, 0x0B, 0x4F00, 0x4F01)           Video info / VESA
    0x16 (0x00, 0x01, 0x02)                     Keyboard input / shift flags
    0x1A (0x00, 0x02, 0x04, 0xB101, 0xB102)     Time, CMOS, PCI BIOS
    */

    printf("Hello world");
    Clear();
    SetCursor(31, 23);
    printf("Press DEL for menu");
    SetCursor(0, 0);
    menu_key = 0x53;
    if (X86_checkForKeys())
    {
        printf("pressed key\n");
        /*
        if (!ELF_Read(&part, "/blmenu.elf", (void**)&menuEntry))
        {
            printf("ELF read failed, booting halted!\n");
            goto end;
            }
            */
        menuEntry(bootParams);
    }
    printf("to kernel\n");
    if (!ELF_Read(&part, "/boot/kernel.elf", (void **)&kernelEntry))
    {
        printf("ELF read failed, booting halted!\n");
        printf("Kernel not found\n");
        goto end;
    }
    readyToJump = true;

    if (readyToJump)
    {
        printf("File table\n");
        fill_table();           // fulls out the tables
        printf("Enable paging and jump\n");
        JumpToKernel(bootParams);         // sets tables and enables PG and jumps
    }

end:
    for (;;)
        ;
}