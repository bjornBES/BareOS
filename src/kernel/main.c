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
#include <boot/bootparams.h>
#include <util/binary.h>
#include "libs/stdio.h"
#include "libs/memory.h"
#include "libs/IO.h"
#include "libs/string.h"
#include "libs/malloc.h"

#include "video/video.h"
#include "video/VGATextDevice.h"
#include "debug/debug.h"
#include "arch/x86/gdt.h"
#include "arch/x86/x86.h"
#include "arch/x86/isr.h"
#include "memory/paging/paging.h"
#include "memory/pmm/pmm.h"
#include "hal/hal.h"
#include "PCI/pci.h"
#include "VFS/vfs.h"
#include "device/device.h"
#include "fs/FAT/FAT.h"
#include "shell/shell.h"
#include "task/loader.h"
#include "ELF/elf.h"
#include "syscall/syscall.h"

#include "drivers/serial/UART/UART.h"
#include "drivers/IO/I8042/I8042.h"
#include "drivers/IO/Keyboard/Keyboard.h"

extern char VGAModesAddr;
extern char default8x16Font;

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
    log_debug("MAIN", "from bootparams @ 0x%llx\n", bootParams);
    log_debug("MAIN", "kernel_address: %p\n", bootParams->kernel_address);
    log_debug("MAIN", "BootDevice: %x\n", bootParams->BootDevice);
    log_debug("MAIN", "currentMode: %x\n", bootParams->currentMode);
    log_debug("MAIN", "pageDirectory: %x\n", bootParams->pageDirectory);
    log_debug("MAIN", "day: %x\n", bootParams->rtc.day);
    log_debug("MAIN", "month: %x\n", bootParams->rtc.month);
    log_debug("MAIN", "year: %x\n", bootParams->rtc.year);
    log_debug("MAIN", "second: %x\n", bootParams->rtc.second);
    log_debug("MAIN", "minute: %x\n", bootParams->rtc.minute);
    log_debug("MAIN", "hour: %x\n", bootParams->rtc.hour);
    log_debug("MAIN", "floppyFlag: %x\n", bootParams->equipment.floppyFlag);
    log_debug("MAIN", "hasCoprocessor: %x\n", bootParams->equipment.hasCoprocessor);
    log_debug("MAIN", "hasFpu: %x\n", bootParams->equipment.hasFpu);
    log_debug("MAIN", "numFloppies: %x\n", bootParams->equipment.numFloppies);
    log_debug("MAIN", "reserved: %x\n", bootParams->equipment.reserved);
    log_debug("MAIN", "vesaModeCount: %x\n", bootParams->vesaModeCount);
    log_debug("MAIN", "e820Count: %x\n", bootParams->e820Count);
    if (UART_init(COM1))
    {
    }
    UART_write_fstr(COM1, "UART has started\r\n");

    log_info("MAIN", "bootParams @ %p", bootParams);
    HALInit();
    UART_write_fstr(COM1, "UART is done\r\n");
    log_debug("MAIN", "Hello world from Kernel");
    enable_interrupts();
    arch_breakpoint();
    paging_init(bootParams);
#ifdef __i686__
    tss_entry.cr3 = (uint32_t)kernel_page;
#endif
    pmm_print_info_verbose();
    log_debug("MAIN", "Paging init");

    log_debug("MAIN", "Init Memory functions");

    log_info("MAIN", "====Moving boot params====");
    paging_print_out = false;
    boot_params *bp = malloc(sizeof(boot_params));
    paging_map_region(kernel_page, (virt_addr)bootParams, (phys_addr)bootParams, sizeof(boot_params) + PAGE_SIZE, PAGE_PRESENT | PAGE_WRITABLE);
    log_debug("MAIN", "Mapped boot params");
    log_debug("MAIN", "Copying from %p to %p", bootParams, bp);
    memcpy(bp, bootParams, sizeof(boot_params));
    log_info("MAIN", "Test the shit new -> old");
    log_info("MAIN", "kernel_address: %p -> %p", bp->kernel_address, bootParams->kernel_address);
    log_info("MAIN", "BootDevice: %x -> %x", bp->BootDevice, bootParams->BootDevice);
    log_info("MAIN", "currentMode: %x -> %x", bp->currentMode, bootParams->currentMode);
    log_info("MAIN", "pageDirectory: %x -> %x", bp->pageDirectory, bootParams->pageDirectory);
    log_info("MAIN", "day: %x -> %x", bp->rtc.day, bootParams->rtc.day);
    log_info("MAIN", "month: %x -> %x", bp->rtc.month, bootParams->rtc.month);
    log_info("MAIN", "year: %x -> %x", bp->rtc.year, bootParams->rtc.year);
    log_info("MAIN", "second: %x -> %x", bp->rtc.second, bootParams->rtc.second);
    log_info("MAIN", "minute: %x -> %x", bp->rtc.minute, bootParams->rtc.minute);
    log_info("MAIN", "hour: %x -> %x", bp->rtc.hour, bootParams->rtc.hour);
    log_info("MAIN", "floppyFlag: %x -> %x", bp->equipment.floppyFlag, bootParams->equipment.floppyFlag);
    log_info("MAIN", "hasCoprocessor: %x -> %x", bp->equipment.hasCoprocessor, bootParams->equipment.hasCoprocessor);
    log_info("MAIN", "hasFpu: %x -> %x", bp->equipment.hasFpu, bootParams->equipment.hasFpu);
    log_info("MAIN", "numFloppies: %x -> %x", bp->equipment.numFloppies, bootParams->equipment.numFloppies);
    log_info("MAIN", "reserved: %x -> %x", bp->equipment.reserved, bootParams->equipment.reserved);
    log_info("MAIN", "vesaModeCount: %x -> %x", bp->vesaModeCount, bootParams->vesaModeCount);
    log_info("MAIN", "e820Count: %x -> %x", bp->e820Count, bootParams->e820Count);
    log_debug("MAIN", "Freeing boot params");
    paging_free_region(kernel_page, (virt_addr)bootParams, sizeof(boot_params) + PAGE_SIZE);
    paging_print_out = true;
    allocator_print_status();

    paging_print_out = false;
    device_init();
    pci_init(bp->pciBios);
    pci_init_devices();
    paging_print_out = true;

    allocator_print_blocks();
    allocator_print_status();

    uint16_t mode = bp->currentMode;
    VESA_mode *vesaMode = NULL;
    log_debug("main", "vesa count = %u", bp->vesaModeCount);
    for (size_t i = 0; i < bp->vesaModeCount; i++)
    {
        VESA_mode *element = &bp->vesaModes[i];
        log_debug("main", "vesa mode %d %dx%dx%d @ %p", element->mode, element->width, element->height, element->bpp, element->frame_buffer);
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

    VFS_init();

    device_debug();

    filesystem *fat_fs = fat_init();
    VFS_register_fs(fat_fs);

    log_info("MAIN", "mounting drives");
    device *ahci;
    log_info("MAIN", "Getting drive 0x101");
    ahci = device_get(0x101); // device 1 partition 1
    log_info("MAIN", "Mounting drive 0x101");
    VFS_mount("/user", ahci);
    log_info("MAIN", "Getting drive 0x100");
    ahci = device_get(0x100); // device 1 partition 0
    log_info("MAIN", "Mounting drive 0x100");
    VFS_mount("/boot", ahci);
    I8042_init();

    Loader_init();
    ELF_init();
    process_init();

    syscall_init();

    log_debug("main", "vesa mode %d %dx%dx%d", vesaMode->mode, vesaMode->width, vesaMode->height, vesaMode->bpp);
    vga_init();
    vga_load_font((uint8_t *)&default8x16Font);
    vga_check();
    video_init(bp, &VGAModesAddr);
    phys_addr fb = (phys_addr)(uint32_64)vesaMode->frame_buffer;
    vga_check();
    // paging_map_page(page_table32, page_table32);
    log_debug("main", "paging %x mapped to %x", fb, fb);
    paging_print_out = false;
    paging_map_region(kernel_page, fb, fb, PAGE_SIZE * 4096, PAGE_PRESENT | PAGE_WRITABLE | PAGE_PCD);
    paging_print_out = true;
    log_debug("main", "paging %x mapped to virt %x/phys %x", fb, paging_get_virtual(kernel_page, fb), fb);
    log_debug("main", "vesa mode %d %dx%dx%d %x", vesaMode->mode, vesaMode->width, vesaMode->height, vesaMode->bpp, vesaMode->frame_buffer);
    vga_clear();

    printf("Hello world");

    int count = 0;
    mount_point **points = vfs_get_mount_points(&count);
    int mount_index = 1;
    mount_point *mnt = points[mount_index - 1];
    log_debug("MAIN", "%u: MOUNT POINT %s IS %s\n", mount_index, mnt->path, mnt->dev->name);
    char mount_path[320];
    strcpy(mount_path, mnt->path);
    char path[320];
    count = sprintf(path, "%s/bin/INIT.ELF", mount_path);
    path[count] = '\0';
    log_debug("MAIN", "PATH = %s\n", path);
    char *argv[16] = {0};
    argv[0] = path;
    process_exec(path, argv);

end:
    // loop
    for (;;)
        ;
}