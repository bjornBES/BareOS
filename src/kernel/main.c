/*
 * File: main.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 May 2026 12:25:55
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
#include "task/threading/priority.h"

#include "video/video.h"
#include "video/VGATextDevice.h"
#include "debug/debug.h"
#include "kernel/setup.h"
#include "time/timer.h"
#include "kernel/exceptions/exception.h"
#include "memory/ioremap/ioremap.h"
#include "memory/paging/paging.h"
#include "memory/pmm/pmm.h"
#include "memory/memdefs.h"
#include "hal/hal.h"
#include "PCI/pci.h"
#include "VFS/vfs.h"
#include "device/device.h"
#include "fs/FAT/FAT.h"
#include "task/loader.h"
#include "ELF/elf.h"
#include "syscall/syscall.h"

#include "task/threading/thread.h"
#include "task/threading/scheduling/scheduler.h"

#include "drivers/serial/UART/UART.h"
#include "drivers/IO/I8042/I8042.h"
#include "drivers/IO/Keyboard/Keyboard.h"

extern char VGAModesAddr;
extern char default8x16Font;

void hexdump(void *ptr, int len)
{
    fprintf(VFS_FD_DEBUG, "from address %p", ptr);
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
            fprintf(VFS_FD_DEBUG, "\n%04x: ", i);
        fprintf(VFS_FD_DEBUG, "%02x ", p[i]);
    }
    fprintf(VFS_FD_DEBUG, "\n");
}

extern void process_user_page_fault(paging_info *info);
void kernel_page_fault(void *_info)
{
    paging_info *info = (paging_info *)_info;
    process_user_page_fault(info);
}

void kernel_breakpoint(registers *regs)
{
    thread *current_thread = scheduler_get_current();
    log_info("breakpoint", "comes from thread %u", current_thread->tid);
}

void test()
{
    log_debug("test", "in function test");
    int t = 0;
    while (t < 10)
    {
        t++;
    }
    log_debug("test", "exiting function test");
    scheduler_thread_exit();
}

void start_init()
{
    int count = 0;
    volume_point **points = vfs_get_volume_points(&count);
    int mount_index = 1;
    volume_point *mnt = points[mount_index - 1];
    log_debug("MAIN", "%u: MOUNT POINT %s IS %s\n", mount_index, mnt->path, mnt->dev->name);
    char mount_path[320];
    strcpy(mount_path, mnt->path);
    char path[320];
    count = sprintf(path, "%s/bin/INIT.ELF", mount_path);
    path[count] = '\0';
    log_debug("MAIN", "PATH = %s\n", path);
    char *argv[16] = {0};
    argv[0] = path;
    kernel_init_process(path, argv, NULL);
}

boot_params *main_boot_params;
thread *main_thread;
__attribute__((noreturn)) void kernel_entry()
{
    exception_register_kernel_handler(EXC_PAGE, kernel_page_fault);
    log_info("MAIN", "main_boot_params @ %p", main_boot_params);
    UART_write_fstr(COM1, "UART is done\r\n");
    log_debug("MAIN", "Hello world from Kernel");

    thread *t = thread_create(test);
    scheduler_add(t);

    main_thread->timeslice = 10;

    allocator_print_status();

    // FADT_shutdown();

    device_init();

    pci_init(main_boot_params->pciBios);
    pci_init_devices();
    VFS_init();

    device_debug();
    {
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
        syscall_init();
    }
    {
        uint16_t mode = main_boot_params->currentMode;
        VESA_mode *vesaMode = NULL;
        log_debug("main", "vesa count = %u", main_boot_params->vesaModeCount);
        for (size_t i = 0; i < main_boot_params->vesaModeCount; i++)
        {
            VESA_mode *element = &main_boot_params->vesaModes[i];
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
        log_debug("main", "vesa mode %d %dx%dx%d", vesaMode->mode, vesaMode->width, vesaMode->height, vesaMode->bpp);
        vga_init();
        vga_load_font((uint8_t *)&default8x16Font);
        vga_check();
        video_init(main_boot_params, &VGAModesAddr);
        vga_clear();
        log_debug("main", "vesa mode %d %dx%dx%d %p", vesaMode->mode, vesaMode->width, vesaMode->height, vesaMode->bpp, vesaMode->frame_buffer);
        printf("Hello world");
    }

    process_init();

    t = thread_create(start_init);
    scheduler_add(t);
    scheduler_sleep_ms(100);
    log_debug("MAIN", "Got back");

    {
        log_debug("MAIN", "loop until something happens main_thread->state = %u", main_thread->state);

        while (main_thread->state != THREAD_DEAD)
        {
        }
        log_crit("MAIN", "main thread died");
        KernelPanic("MAIN", "main thread died");
    }
end:
    // loop
    for (;;)
        ;
}

void main(boot_params *bootParams)
{
    if (UART_init(COM1))
    {
    }
    UART_write_fstr(COM1, "UART has started\r\n");
    ioremap_init();

    kstack_init();

    paging_print_out = false;
    main_boot_params = setup_arch(bootParams);
    HALInit();

    log_debug("MAIN", "init main thread");
    allocator_print_blocks();
    main_thread = thread_create_main();

    log_debug("MAIN", "new rsp = %p", main_thread->stack_base);
    __asm__("mov rsp, %0" : : "r"((uint32_64)main_thread->stack_base));

    scheduler_init(main_thread);
    log_debug("MAIN", "main thread stack @ %p", main_thread->stack_base);
    __asm__("jmp kernel_entry");
    __builtin_unreachable();
}
