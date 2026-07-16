/*
 * File: main.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdint.h>
#include <boot/bootparams.h>
#include <util/binary.h>
#include "stdio.h"
#include "kernel/io.h"
#include "task/threading/priority.h"

#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/setup.h"
#include "kernel/mmu.h"
#include "kernel/cpu.h"
#include "kernel/smp.h"

#include "task/threading/thread_type.h"
#include "debug/debug.h"
#include "time/timer.h"
#include "mm/ioremap/ioremap.h"
#include "mm/pmm.h"
#include "mm/memdefs.h"
#include "PCI/pci.h"
#include "VFS/vfs.h"
#include "device/device.h"
#include "fs/FAT/FAT.h"
#include "task/loader.h"
#include "ELF/elf.h"
#include "syscall/syscall.h"

#include "task/threading/thread.h"
#include "task/threading/scheduling/scheduler.h"

#include "drivers/video/video.h"
#include "drivers/serial/UART/UART.h"
#include "drivers/IO/tty/tty.h"
#include "drivers/IO/tty/tty_flags.h"

extern char default8x16Font;

void hexdump(void *ptr, int len)
{
    fprintf(VFS_FD_DEBUG, "========= HEXDUMP =========\n");
    fprintf(VFS_FD_DEBUG, "hexdump at %p length %u\n", ptr, len);
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
        {
            fprintf(VFS_FD_DEBUG, "\n%04x: ", i);
        }
        fprintf(VFS_FD_DEBUG, "%02x ", p[i]);
    }
    fprintf(VFS_FD_DEBUG, "\n");
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
    char *argv[2] = {"/user:/bin/INIT.ELF", NULL};
    process_exec("/user:/bin/INIT.ELF", argv, NULL, NULL);
}

boot_params_t *main_boot_params;
thread_t *main_thread;

__attribute__((noreturn)) void kernel_entry()
{
    device_init();

    tty_struct_t *stdin = NULL;
    tty_struct_t *stdout = NULL;
    {
        video_init(main_boot_params);
        device_t *vga_dev = device_get_by_id(DEVICE_VIDEO, 1);

        vfs_init();

        stdin = tty_create(device_get_by_name("kbd0"), NULL);
        termios_t stdin_term;
        stdin_term.c_lflag &= ~(ISIG | ECHO | ICRNL);
        stdin_term.c_lflag |= ICANON;
        tty_termios_set(stdin, &stdin_term);

        stdout = tty_create(NULL, vga_dev);
        /* tty_struct_t *stderr = */ tty_create(NULL, vga_dev);

        device_ioctl(vga_dev, VIDEO_IOCTL_CLEAR, NULL);
        tty_write(stdout, (const uint8_t *)"VGA is done\n", 12);
    }

    UART_init(COM1);
    termios_t uart_term = {0};
    tty_struct_t *uart = tty_create(NULL, device_get_by_name("uart0"));
    tty_baudrate_encode_baud_rate(&uart_term, 0, 38400);
    uart_term.c_cflag |= CS8;
    uart_term.c_oflag |= ONLCR;
    tty_termios_set(uart, &uart_term);
    tty_write(uart, (const uint8_t *)"UART is done\n", 13);

    log_info("MAIN", "main_boot_params @ %p", main_boot_params);
    log_debug("MAIN", "Hello world from Kernel");
    // for (;;);

    thread_t *t = thread_create(test);
    scheduler_add(t);

    main_thread->timeslice = 10;

    allocator_print_status();

    // fadt_shutdown();

    pci_init();
    pci_init_devices();

    device_debug();
    {
        fat_init();

        log_info("MAIN", "mounting drives");
        device_t *ahci;
        log_info("MAIN", "Getting drive 0x101");
        ahci = device_get_by_name("sata1_13"); // device 1 partition 1
        log_info("MAIN", "Mounting drive %p", ahci);
        vfs_mount("/user:/", ahci, 0);
        log_info("MAIN", "Getting drive 0x100");
        ahci = device_get_by_name("sata1_02"); // device 1 partition 0
        log_info("MAIN", "Mounting drive 0x100");
        vfs_mount("/boot:/boot", ahci, 0);

        fd_t file = vfs_open("/user:/home/test.txt", 0, 0);
        uint8_t data[512];
        vfs_read(file, data, 512);
        hexdump(data, 512);

        Loader_init();
        ELF_init();
        syscall_init();
    }

    process_init();

    t = thread_create(start_init);
    scheduler_add(t);
    scheduler_sleep_ms(100);
    log_debug("MAIN", "Got back");

    {
        log_debug("MAIN",
                  "loop until something happens main_thread->state = %u",
                  main_thread->state);

        while (main_thread->state != THREAD_DEAD)
        {
        }
        log_crit("MAIN", "main thread died");
        KernelPanic("MAIN", "main thread died");
    }
    // loop
    for (;;);
}

void kernel_main(boot_params_t *bootParams)
{
    hexdump(bootParams, sizeof(boot_params_t));

    log_debug(NO_MODULE, "from bootparams @ %p", bootParams);
    log_debug(NO_MODULE, "kernel_address: %p", bootParams->kernel_address);
    log_debug(NO_MODULE, "BootDevice: %x", bootParams->boot_device);
    log_debug(NO_MODULE, "currentMode: %x", bootParams->current_mode);
    log_debug(NO_MODULE, "e820Count: %x", bootParams->memory.count);
    log_debug(NO_MODULE, "boot_flags: %x", bootParams->bootloader.boot_flags);
    log_debug(NO_MODULE, "vesaModeCount: %x", bootParams->video.count);
    log_debug(NO_MODULE, "rsdp_address: %p", bootParams->acpi.rsdp_address);

    ioremap_init();
    kstack_init();

    main_boot_params = setup_arch(bootParams);

    smp_arch_init(main_boot_params);

    timer_init(main_boot_params);

    // HALInit();

    log_debug("MAIN", "init main thread");
    allocator_print_blocks();
    main_thread = thread_create_main();

    log_debug("MAIN", "new rsp = %p", main_thread->kernel_stack);
    __asm__("mov rsp, %0" : : "r"((uint32_64)main_thread->kernel_stack));

    scheduler_init(main_thread);
    log_debug("MAIN", "main thread stack @ %p", main_thread->kernel_stack);
    __asm__("jmp kernel_entry");
    __builtin_unreachable();
}
