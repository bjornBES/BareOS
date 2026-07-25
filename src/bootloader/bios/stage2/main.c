/*
 * File: main.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
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
#include "bios/bios.h"
#include "memdefs.h"
#include "partition/gpt.h"
#include "partition/partition.h"
#include "fs/disk.h"
#include "fs/fat.h"
#include "elf/elf.h"
#include "cpuID/CPUID.h"
#include "bootparams/memory/memdetect.h"
#include "bootparams/video/vesa.h"
#include "bootparams/ACPI/ACPI.h"
#include "menu/menu.h"
#include <time.h>

typedef void (*BootStart)(boot_params_t *boot);

BootStart kernelEntry;
boot_params_t bss_bootParams;

extern uint16_t BootPartitionSeg;
extern uint16_t BootPartitionOff;
extern char __trampoline_start;

void hexdump(void *ptr, int len)
{
    printf("========= HEXDUMP =========\n");
    printf("hexdump at 0x%p length %u\n", ptr, len);
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
        {
            printf("\n%x: ", i);
        }
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    boot_params_t *bootParams = (boot_params_t *)MEMORY_BOOTPARAMS_ADDR;
    printf("bootParams @ 0x%x\n", bootParams);
#pragma GCC diagnostic pop

    hexdump(bootParams, 512);
    printf("from bootparams @ 0x%x\n", bootParams);
    /*     hexdump((uint8_t*)&__trampoline_start, 512);
        hexdump(bootParams->smp_trampoline, 512); */

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

    /*     printf("Hello world");
        vga_set_cursor(31, 23);
        printf("Press DEL for menu");
        vga_set_cursor(0, 0);
        menu_key = 0x53;
        if (X86_checkForKeys())
        {
            printf("pressed key\n");
            menuEntry(bootParams);
        }
        printf("to kernel\n"); */

    memset(bootParams, 0, sizeof(boot_params_t));
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
    bootParams->current_mode = 0x115;

    {
        biosregs in_regs;
        biosregs out_regs;
        memset(&in_regs, 0, sizeof(biosregs));
        memset(&out_regs, 0, sizeof(biosregs));

        in_regs.ah = 0x04;
        intcall(0x1A, &in_regs, &out_regs);
        uint8_t century = 0;
        if (BIT_GET(out_regs.flags, X86_EFLAGS_CF_BIT))
        {
            if (out_regs.ch == 0)
            {
                century = 20;
            }
        }
        else
        {
            century = out_regs.ch;
        }
        uint8_t year = out_regs.cl;
        uint8_t day = out_regs.dl;
        uint8_t month = out_regs.dh;

        memset(&in_regs, 0, sizeof(biosregs));
        memset(&out_regs, 0, sizeof(biosregs));

        in_regs.ah = 0x02;
        intcall(0x1A, &in_regs, &out_regs);
        if (BIT_GET(out_regs.flags, X86_EFLAGS_CF_BIT))
        {
        }
        uint8_t hour = out_regs.ch;
        uint8_t minute = out_regs.cl;
        uint8_t second = out_regs.dh;
        bootParams->boot_time = bcd_time_to_unix(century, year, month, day, hour, minute, second);
    }
    
    for (size_t i = 0; i < 512; i++)
    {
        bootParams->cpu_core_trampoline[i] = *((uint8_t *)&__trampoline_start + i);
    }

    bootParams->page_directory = 0x112255AA;
    bootParams->boot_device = bootDrive;

    bootParams->bootloader.boot_flags = 1;
    strcpy(bootParams->bootloader.bootloader_name, "BESOS Bootloader");
    strcpy(bootParams->bootloader.cmd_line, "");

    DetectVESA(bootParams);
    DetectACPI(bootParams);
    DetectMemory(bootParams);

    printf("from bootparams @ 0x%x\n", bootParams);
    printf("kernel_address: %p\n", bootParams->kernel_address);
    printf("BootDevice: %x\n", bootParams->boot_device);
    printf("currentMode: %x\n", bootParams->current_mode);
    printf("e820Count: %x\n", bootParams->memory.count);
    printf("boot_flags: %x\n", bootParams->bootloader.boot_flags);
    printf("vesaModeCount: %x\n", bootParams->video.count);
    printf("rsdp_address: %p\n", bootParams->acpi.rsdp_address);

    hexdump(bootParams, 512);

    cpuid_regs reg;
    INIT_CPUID_REG(&reg);
    CPUID(0x80000001, 0, &reg);
    if (BIT_GET(reg.edx, 29))
    {
        bootParams->stage.mode = USE_64_BIT_MODE;
        printf("To 64 bit mode\n");

        printf("from bootparams @ 0x%x\n", bootParams);
        printf("kernel_address: %p\n", bootParams->kernel_address);
        printf("BootDevice: %x\n", bootParams->boot_device);
        printf("currentMode: %x\n", bootParams->current_mode);
        printf("e820Count: %x\n", bootParams->memory.count);
        printf("boot_flags: %x\n", bootParams->bootloader.boot_flags);
        printf("vesaModeCount: %x\n", bootParams->video.count);
        printf("rsdp_address: %p\n", bootParams->acpi.rsdp_address);

        x86_EnterLongMode();
    }
    else
    {
        bootParams->stage.mode = USE_32_BIT_MODE;
    }

    if (readyToJump)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
        bootParams->page_directory = (uint32_t)(uint32_t *)(((void *)&page_directory_table) + KERNEL_VMA);
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
    for (;;);
}
