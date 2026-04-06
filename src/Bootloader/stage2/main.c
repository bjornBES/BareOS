/*
 * File: main.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdint.h>
#include <stdbool.h>
#include <boot/bootparams.h>
#include <util/binary.h>
#include "stdio.h"
#include <stddef.h>
#include "memory.h"
#include "string.h"

#include "paging/paging.h"
#include "x86.h"
#include "memdefs.h"
#include "fs/mbr.h"
#include "fs/disk.h"
#include "fs/fat.h"
#include "elf/elf.h"
#include "bootparams/memory/memdetect.h"
#include "bootparams/pci/pci.h"
#include "bootparams/video/vesa.h"
#include "bootparams/equipment/CPUID.h"
#include "bootparams/equipment/Equipment.h"
#include "bootparams/ACPI/ACPI.h"
#include "menu/menu.h"

typedef void (*BootStart)(boot_params *boot);

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

void __attribute__((cdecl)) start(uint32_t bootDrive, void *partition)
{
    bool readyToJump = false;
    printf("%x", partition);
    hexdump(partition, 64);

    boot_params *bootParams = (boot_params *)MEMORY_BOOTPARAMS_ADDR;

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

    bootParams->bootLoader.bootFlags = 1;
    strcpy(bootParams->bootLoader.bootloaderName, "BESOS Bootloader");
    strcpy(bootParams->bootLoader.cmdline, "");

    DetectMemory(bootParams);
    DetectVESA(bootParams);
    DetectPCI(bootParams);
    DetectEquipment(bootParams);
    DetectCPUID(bootParams);
    DetectACPI(bootParams);
    

    printf("Hello world");
    vga_clear();
    vga_set_cursor(31, 23);
    printf("Press DEL for menu");
    vga_set_cursor(0, 0);
    menu_key = 0x53;
    if (X86_checkForKeys())
    {
        printf("pressed key\n");
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

    x86_SetVESAMode(0x115);
    bootParams->currentMode = 0x115;

/*
Set the PAE enable bit in CR4
Load CR3 with the physical address of the PML4 (Level 4 Page Map)
Enable long mode by setting the LME flag (bit 8) in MSR 0xC0000080 (aka EFER)
Enable paging
*/

    cpuid_regs reg;
    INIT_CPUID_REG(&reg);
    CPUID(0x80000001, 0, &reg);

    if (BIT_GET(reg.edx, 29))
    {
        strcpy(bootParams->bootLoader.cmdline, "LM");
        x86_EnterLongMode();
    }
    else
    {
        strcpy(bootParams->bootLoader.cmdline, "PM");
    }

    if (readyToJump)
    {
        bootParams->pageDirectory = (uint64_t*)(uint32_t*)((void*)page_directory_table + KERNEL_VMA);
        fill_32bit_table();
        printf("jump to 0x%p\n", *kernelEntry);
        __asm__("cli"); // dont ask.
        __asm__("mov cr3, eax" : : "a"((uint32_t)page_directory_table));
        __asm__("mov eax, cr0");
        __asm__("or eax, 0x80000000");
        __asm__("mov cr0, eax");
        __asm__("sti");
        __asm__("mov edi, %0" : : "r"(bootParams) );
        __asm__("push 0x08" );
        __asm__("push %0" : : "r"(kernelEntry) );
        __asm__("retf" );

        __builtin_unreachable();
    }

end:
    for (;;)
        ;
}