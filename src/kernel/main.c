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
#include "arch/i686/gdt.h"
#include "arch/i686/i686.h"
#include "arch/i686/isr.h"
#include "hal/hal.h"
#include "paging/paging.h"
#include "paging/frame.h"
#include "PCI/pci.h"
#include "VFS/vfs.h"
#include "malloc.h"
#include "device/device.h"
#include "fs/FAT/FAT.h"
#include "shell/shell.h"
#include "task/loader.h"
#include "ELF/elf.h"

#include "drivers/serial/UART/UART.h"
#include "drivers/IO/I8042/I8042.h"
#include "drivers/IO/Keyboard/Keyboard.h"

extern char __end;
extern char __kernel_end;
extern char __kernel_start;
extern char __heap_size;
extern char VGAModesAddr;
extern char default8x16Font;

void crash_me();

void PageFault(Registers *regs)
{
    uint32_t cr2;
    __asm__("movl %%cr2,%0" : "=rm"(cr2));

    fprintf(VFS_FD_DEBUG, "[Page Fault] Virtual address %p\n", cr2);

    fprintf(VFS_FD_DEBUG, "[Page Fault] Unhandled Page Fault %d\n", regs->interrupt);
    fprintf(VFS_FD_DEBUG, "[Page Fault]   eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    fprintf(VFS_FD_DEBUG, "[Page Fault]   esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x\n", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->es, regs->ss);

    fprintf(VFS_FD_DEBUG, "[Page Fault] Error code 0x%x\n", regs->error);
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Present %u\n", BIT_GET(regs->error, 0));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Write %u\n", BIT_GET(regs->error, 1));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    User %u\n", BIT_GET(regs->error, 2));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Reserved write %u\n", BIT_GET(regs->error, 3));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Instruction Fetch %u\n", BIT_GET(regs->error, 4));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Protection key %u\n", BIT_GET(regs->error, 5));

    UART_write_fstr(COM1, "[Page Fault] Virtual address %p\r\n", cr2);

    UART_write_fstr(COM1, "[Page Fault] Unhandled Page Fault %d\r\n", regs->interrupt);
    UART_write_fstr(COM1, "[Page Fault]   eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\r\n", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    UART_write_fstr(COM1, "[Page Fault]   esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x es=%x ss=%x\r\n", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->es, regs->ss);

    UART_write_fstr(COM1, "[Page Fault] Error code 0x%x\r\n", regs->error);
    UART_write_fstr(COM1, "[Page Fault]    Present %u\r\n", BIT_GET(regs->error, 0));
    UART_write_fstr(COM1, "[Page Fault]    Write %u\r\n", BIT_GET(regs->error, 1));
    UART_write_fstr(COM1, "[Page Fault]    User %u\r\n", BIT_GET(regs->error, 2));

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

    HALInit();
    // uint8_t* addr2 = (uint8_t*)0x1000000;
    // *addr2 = 10;

    i686_isr_register_handler(14, PageFault);

    enableInterrupts();
    log_debug("MAIN", "Hello world from Kernel");
    kernel_page_directory = (page_directory_entry*)bootParams->pageDirectory;
    tss_entry.cr3 = (uint32_t)kernel_page_directory;
    paging_init();

    log_warn("MAIN", "=========== nr.1 ===========");
    frame_dump_bitmap();
    uint32_t kernel_start_virt = (uint32_t)&__kernel_start;
    uint32_t kernel_end_virt = (uint32_t)&__kernel_end;
    uint32_t kernel_start_phys = kernel_start_virt - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE;
    uint32_t kernel_end_phys = kernel_end_virt - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE;
    uint32_t kernel_size = kernel_end_phys - kernel_start_phys;
    log_debug("Main", "kernel virt start %p - end %p", kernel_start_virt, kernel_end_virt);
    log_debug("Main", "kernel phys start %p - end %p", kernel_start_phys, kernel_end_phys);

    // What ever past me took i want it now
    // frame_alloc_region(kernel_start_virt, kernel_start_phys);
    // - BjornBEs 19-03-2026
    paging_map_region(kernel_page_directory, (void*)kernel_start_virt, (void*)kernel_start_phys, kernel_size, -1);
log_warn("MAIN", "=========== nr.2 ===========");
    frame_dump_bitmap();
    log_debug("MAIN", "if you can see this it worked now fuck you");
    uint32_t heap_start = (uint32_t)&__end;
    log_debug("MAIN", "if you can see this it worked now fuck you");
    uint32_t heap_start_phys = heap_start - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE;
    size_t heap_size = (size_t)&__heap_size;
    uint32_t heap_end_phys = heap_start_phys + heap_size;
    log_debug("Main", "heap_start at %p phys %p", heap_start, heap_start_phys);
    frame_alloc_region(heap_start_phys, heap_end_phys);
    // paging_map_region(kernel_page_directory, (void*)heap_start_phys, (void*)heap_start_phys, heap_size, -1);

    // frame_alloc_region(kernel_start_phys, kernel_end_phys);
    log_debug("MAIN", "Kernel phys %x-%x virt %x-%x size %x/%x", kernel_start_phys, kernel_end_phys, kernel_start_virt, kernel_end_virt, kernel_size, kernel_end_phys - kernel_start_phys);

    log_debug("MAIN", "Init Memory functions");
    int heap_page = GETPAGEDIRECTORYINDEX(heap_start);
    mmInit((uint32_t)heap_start, heap_size);
    mmPrintStatus();

    boot_params *bp = malloc(sizeof(boot_params));
    memcpy(bp, bootParams, sizeof(boot_params));

    device_init();

    pci_init(bp->pciBios);
    pci_init_devices();

    mmPrintBlocks();
    mmPrintStatus();

    uint16_t mode = bp->currentMode;
    VESA_mode *vesaMode = NULL;
    log_debug("main", "vesa count = %u", bp->vesaModeCount);
    for (size_t i = 0; i < bp->vesaModeCount; i++)
    {
        VESA_mode *element = &bp->vesaModes[i];
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
    video_init(bp, &VGAModesAddr);
    uint32_t *fb = (uint32_t *)vesaMode->frame_buffer;

    // paging_map_page(page_table, page_table);
    log_debug("main", "paging %x mapped to %x", fb, fb);
    paging_map_region(kernel_page_directory, (void *)fb, (void *)fb, PAGE_SIZE * 4096, PAGE_PRESENT | PAGE_WRITABLE);
    log_debug("main", "paging %x mapped to virt %x/phys %x", fb, paging_get_virtual(kernel_page_directory, fb), paging_get_physical(kernel_page_directory, fb));
    log_debug("main", "vesa mode %d %dx%dx%d %x", vesaMode->mode, vesaMode->width, vesaMode->height, vesaMode->bpp, vesaMode->frame_buffer);

    VFS_init();

    filesystem *fat_fs = fat_init();
    VFS_register_fs(fat_fs);

    device *ahci = device_get(0x100); // device 1 partition 0
    VFS_mount("/boot", ahci);
    ahci = device_get(0x101); // device 1 partition 1
    VFS_mount("/", ahci);

    I8042_init();

    /*     fd_t file = VFS_open("/test.txt");
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
        VFS_close(file); */

    // hexdump(buffer, 512);

    Loader_init();
    ELF_init();

    printf("Hello world");

    shell_enter();

end:
    // loop
    for (;;)
        ;
}