/*
 * File: main.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdio.h>
#include <stdint.h>
#include <boot/bootparams.h>
#include <memory.h>
#include <IO.h>
#include <util/binary.h>
#include <string.h>
#include <core/video/video.h>
#include <core/video/VGATextDevice.h>

#include "debug/debug.h"
#include "drivers/serial/UART/UART.h"
#include "arch/i686/i686.h"
#include "arch/i686/isr.h"
#include "hal/hal.h"
#include "paging/paging.h"
#include "PCI/pci.h"
#include "VFS/vfs.h"
#include "malloc.h"
#include "device/device.h"
#include "fs/FAT/FAT.h"

extern char __end;
extern char VGAModesAddr;
extern char default8x16Font;

void crash_me();

void PageFault(Registers *regs)
{
    uint32_t cr2;
    __asm__("movl %%cr2,%0" : "=rm"(cr2));
    UART_write_fstr(COM1, "[Page Fault] Virtual address %p\r\n", cr2);

    UART_write_fstr(COM1, "[Page Fault] Unhandled Page Fault %d\r\n", regs->interrupt);
    UART_write_fstr(COM1, "[Page Fault]   eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\r\n", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    UART_write_fstr(COM1, "[Page Fault]   esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x\r\n", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);

    UART_write_fstr(COM1, "[Page Fault] Error code 0x%x\r\n", regs->error);
    UART_write_fstr(COM1, "[Page Fault]    Present %s\r\n", BIT_GET(regs->error, 0) ToBoolString);
    UART_write_fstr(COM1, "[Page Fault]    Write %s\r\n", BIT_GET(regs->error, 1) ToBoolString);
    UART_write_fstr(COM1, "[Page Fault]    User %s\r\n", BIT_GET(regs->error, 2) ToBoolString);

    log_crit("Page Fault", "Virtual address %p", cr2);

    log_info("Page Fault", " Unhandled Page Fault %d", regs->interrupt);
    log_info("Page Fault", "   eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    log_info("Page Fault", "   esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->U16.ds, regs->U16.es, regs->ss);

    log_info("Page Fault", " Error code 0x%x", regs->error);
    log_info("Page Fault", "    Present %s", BIT_GET(regs->error, 0) ToBoolString);
    log_info("Page Fault", "    Write %s", BIT_GET(regs->error, 1) ToBoolString);
    log_info("Page Fault", "    User %s", BIT_GET(regs->error, 2) ToBoolString);

for (; ;)
;

    KernelPanic("Page Fault", "Got page fault from %x", cr2);
}

void hexdump(void *ptr, int len)
{
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
            fprintf(VFS_FD_DEBUG, "\n%04x: ", i);
        fprintf(VFS_FD_DEBUG, "%02x ", p[i]);
    }
    fprintf(VFS_FD_DEBUG, "\n");
}

void main(boot_params *bootParams)
{
    if (UART_init(COM1))
    {
    }
    vga_clear();

    HALInit();
    // uint8_t* addr2 = (uint8_t*)0x1000000;
    // *addr2 = 10;

    i686_ISRRegisterHandler(14, PageFault);

    page_directory = (page_directory_entry *)bootParams->pageDirectory;

    enableInterrupts();
    log_debug("MAIN", "Hello world from Kernel");
    log_debug("MAIN", "Start time at");
    log_debug("MAIN", "   %x-%x-%x", bootParams->rtc.day, bootParams->rtc.month, bootParams->rtc.year);
    log_debug("MAIN", "   %x:%x:%x", bootParams->rtc.hour, bootParams->rtc.minute, bootParams->rtc.second);
    log_debug("MAIN", "ACPI");
    log_debug("MAIN", "   id = %x address = %x", bootParams->acpi.lapicId, bootParams->acpi.rsdpAddress);
    // log_debug("MAIN", "page_directory = %p", page_directory);

    BootLoaderInfo BLInfo = bootParams->bootLoader;
    log_debug("MAIN", "Bootloader %s Info", BLInfo.bootloaderName);
    log_debug("MAIN", "Flags = %d", BLInfo.bootFlags);
    log_debug("MAIN", "Cl args = %s", BLInfo.cmdline);

    log_debug("MAIN", "E820 Entries %u count\n", bootParams->e820Count);
    E820_entry *page_table = NULL;
    {
        char *memoryType[5];
        memoryType[0] = "AVAILABLE";
        memoryType[1] = "RESERVED";
        memoryType[2] = "ACPI_RECLAIMABLE";
        memoryType[3] = "NVS";
        memoryType[4] = "BADRAM";
#define _4GB 4294967292
        for (size_t i = 0; i < bootParams->e820Count; i++)
        {
            E820_entry entry = bootParams->e820Entries[i];
            log_debug("MAIN", "\t%u: 0x%llx 0x%llx %u %s", i, entry.addr, entry.size, entry.type, memoryType[entry.type - 1]);
            if (entry.type == MEMORY_AVAILABLE && page_table == NULL)
            {
                if (entry.addr < _4GB && entry.size >= 1024 * 1024) // 1 Mb
                {
                    log_debug("main", "found entry for page table %u", i);
                    page_table = &bootParams->e820Entries[i];
                }
                else
                {
                    // for 64 bit systems
                }
            }
        }
#undef _4GB
    }

    // Breakpoint();
    log_debug("main", "page table address 0x%llx", page_table->addr);
    uint32_t *address = (uint32_t *)(uint32_t)page_table->addr;
    paging_init(address);

    log_debug("MAIN", "Init Memory functions");
    uint8_t *heap = (uint8_t *)&__end;
    log_debug("Main", "heap at %p", heap);
    int heap_page = GETPAGEDIRECTORYINDEX(heap);
    if (!paging_check_page(heap_page))
    {
        paging_mark_page_used(heap_page);
        paging_map_region(heap, heap, 1024, -1);
    }
    mmInit((uint32_t)heap, 1024 * 256);
    mmPrintStatus();

    device_init();

    pci_init(bootParams->pciBios);
    pci_init_devices();

    mmPrintBlocks();
    mmPrintStatus();

    uint16_t mode = bootParams->currentMode;
    VESA_mode *vesaMode = NULL;
    log_debug("main", "vesa count = %u", bootParams->vesaModeCount);
    for (size_t i = 0; i < bootParams->vesaModeCount; i++)
    {
        VESA_mode *element = &bootParams->vesaModes[i];
        log_debug("main", "is %i == %i", element->mode, mode);
        if (element->mode == mode)
        {
            vesaMode = element;
            break;
        }
    }
    if (vesaMode == NULL)
    {
        log_crit("main", "Something is fucked");
    }

    log_debug("main", "vesa mode %d %dx%dx%d", vesaMode->mode, vesaMode->width, vesaMode->height, vesaMode->bpp);

    vga_init();
    vga_load_font((uint8_t *)&default8x16Font);

    video_init(bootParams, &VGAModesAddr);
    uint32_t *fb = (uint32_t *)vesaMode->frame_buffer;

    // paging_map_page(page_table, page_table);
    paging_mark_page_used(GETPAGEDIRECTORYINDEX(fb));
    paging_map_region((void *)fb, (void *)fb, 512, -1);

    /*     for (size_t y = 0; y < 5; y++)
        {
            for (size_t x = 0; x < vesaMode->width; x++)
            {
                video_set_pixel(x, y, 0x00FF0000);
            }
        } */

    device *ahci = device_get(0x100); // device 1 partition 0

    VFS_init();

    filesystem *fat_fs = fat_init();
    VFS_register_fs(fat_fs);

    VFS_mount("/", ahci);

    fd_t file = VFS_open("/test.txt");
    log_debug("MAIN", "got a file descriptor from /test.txt %u", file);
    uint8_t *buffer = malloc(512);
    VFS_read(file, buffer, 512);
    log_debug("MAIN", "read /test.txt got %u", file);
    VFS_close(file);
    log_debug("MAIN", "closed /test.txt got %u", file);
    log_debug("MAIN", "===================================");
    file = VFS_open("/test/hello.txt");
    log_debug("MAIN", "got a file descriptor from /test/hello.txt %u", file);
    VFS_read(file, buffer, 512);
    VFS_close(file);

    hexdump(buffer, 512);

    end:
    // loop
    for (;;)
        ;
}