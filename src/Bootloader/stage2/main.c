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
#include "partition/gpt.h"
#include "partition/partition.h"
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
boot_params bss_bootParams;

extern uint16_t BootPartitionSeg;
extern uint16_t BootPartitionOff;

void hexdump(void *ptr, int len)
{
    printf("========= HEXDUMP =========\n");
    printf("hexdump at 0x%p length %u\n", ptr, len);
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
    printf("\n======================= Start =======================\n");
    bool readyToJump = false;
    printf("partition = %x\n", partition);
    if (partition == NULL)
    {
        printf("new partition seg:off = %x:%x\n", BootPartitionSeg, BootPartitionOff);
        partition = segoffset_to_linear_real(BootPartitionSeg, BootPartitionOff);
        printf("new partition = %x\n", partition);
    }
    hexdump(partition, 64);

    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive))
    {
        printf("Disk init error\r\n");
        goto end;
    }

    Partition part;
    MBR_detect_partition(&part, &disk, partition);


    if (!FAT_Initialize(&part))
    {
        printf("FAT init error\r\n");
        goto end;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    boot_params *bootParams = (boot_params *)MEMORY_BOOTPARAMS_ADDR;
    printf("bootParams @ 0x%x\n", bootParams);
#pragma GCC diagnostic pop
    memset(bootParams, 0, sizeof(boot_params));
    bootParams->pageDirectory = 0x112255AA;
    bootParams->BootDevice = bootDrive;

    bootParams->bootLoader.bootFlags = 1;
    strcpy(bootParams->bootLoader.bootloaderName, "BESOS Bootloader");
    strcpy(bootParams->bootLoader.cmdline, "");

    DetectMemory(bootParams);
    DetectVESA(bootParams);
    DetectPCI(bootParams);
    DetectEquipment(bootParams);
    DetectCPUID(bootParams);
    // DetectACPI(bootParams);

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
    disableOutput = true;
    if (!ELF_Read(&part, "/boot/kernel.elf", (void **)&kernelEntry, bootParams))
    {
        printf("ELF read failed, booting halted!\n");
        printf("Kernel not found\n");
        goto end;
    }
    disableOutput = false;
    readyToJump = true;

    x86_SetVESAMode(0x115);
    bootParams->currentMode = 0x115;

    printf("from bootparams @ 0x%llx\n", bootParams);
    printf("kernel_address: %p\n", bootParams->kernel_address);
    printf("BootDevice: %x\n", bootParams->BootDevice);
    printf("currentMode: %x\n", bootParams->currentMode);
    printf("pageDirectory: %x\n", bootParams->pageDirectory);
    printf("day: %x\n", bootParams->rtc.day);
    printf("month: %x\n", bootParams->rtc.month);
    printf("year: %x\n", bootParams->rtc.year);
    printf("second: %x\n", bootParams->rtc.second);
    printf("minute: %x\n", bootParams->rtc.minute);
    printf("hour: %x\n", bootParams->rtc.hour);
    printf("floppyFlag: %x\n", bootParams->equipment.floppyFlag);
    printf("hasCoprocessor: %x\n", bootParams->equipment.hasCoprocessor);
    printf("hasFpu: %x\n", bootParams->equipment.hasFpu);
    printf("numFloppies: %x\n", bootParams->equipment.numFloppies);
    printf("reserved: %x\n", bootParams->equipment.reserved);
    printf("vesaModeCount: %x\n", bootParams->vesaModeCount);
    printf("e820Count: %x\n", bootParams->e820Count);

    cpuid_regs reg;
    INIT_CPUID_REG(&reg);
    CPUID(0x80000001, 0, &reg);
#ifdef __x86_64__
    if (BIT_GET(reg.edx, 29))
    {
        // strcpy(bootParams->bootLoader.cmdline, "LM");
        printf("To 64 bit mode\n");
        x86_EnterLongMode();
    }
    else
#endif
    {
        // strcpy(bootParams->bootLoader.cmdline, "PM");
    }

    if (readyToJump)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
        bootParams->pageDirectory = (uint32_t)(uint32_t *)(((void *)&page_directory_table) + KERNEL_VMA);
#pragma GCC diagnostic pop
        fill_32bit_table();
        printf("jump to 0x%p\n", *kernelEntry);
        __asm__("cli"); // dont ask.
        __asm__("mov cr3, eax" : : "a"((uint32_t)&page_directory_table));
        __asm__("mov eax, cr0");
        __asm__("or eax, 0x80000000");
        __asm__("mov cr0, eax");
        __asm__("sti");
        __asm__("mov edi, %0" : : "r"(bootParams));
        __asm__("push 0x08");
        __asm__("push %0" : : "r"(kernelEntry));
        __asm__("retf");

        __builtin_unreachable();
    }

end:
    for (;;)
        ;
}