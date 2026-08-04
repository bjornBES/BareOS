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
#include "pci/pci.h"
#include "VFS/vfs.h"
#include "VFS/vfs_flags.h"
#include "device/device.h"
#include "fs/FAT/FAT.h"
#include "fs/devfs/devfs.h"
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

uintptr_t start_init(void *_)
{
    char *argv[2] = {"/user!/bin/INIT.ELF", NULL};
    process_exec("/user!/bin/INIT.ELF", argv, NULL, NULL, NULL);
    return 1;
}

void wait_loop()
{
    while (true)
    {
    }
}

boot_params_t *main_boot_params;
thread_t *main_thread;

__attribute__((noreturn)) void kernel_entry()
{
    irq_arch_disable();
    device_init();
    devfs_init();
    vfs_init();

    setup_arch_post();

    UART_init(COM1);
    tty_struct_t *stdin = NULL;
    tty_struct_t *stdout = NULL;
    {
        video_init(main_boot_params);
        device_t *vga_dev = device_get_by_id(DEVICE_VIDEO, 1);

        stdin = tty_create(device_get_by_name("kbd0"), vga_dev);
        stdin->winsize.ws_col = 80;
        stdin->winsize.ws_row = 25;
        termios_t stdin_term;
        stdin_term.c_lflag &= ~(ISIG);
        stdin_term.c_lflag |= ICANON | ECHO | ICRNL;
        tty_termios_set(stdin, &stdin_term);

        stdout = tty_create(NULL, vga_dev);
        /* tty_struct_t *stderr = */ tty_create(NULL, vga_dev);

        device_ioctl(vga_dev, VIDEO_IOCTL_CLEAR, NULL);
    }

    termios_t uart_term = {0};
    tty_struct_t *stddebug = tty_create(NULL, device_get_by_name("uart0"));
    /* tty_struct_t *stddebug2 = */ tty_create(NULL, device_get_by_name("debug0"));
    tty_baudrate_encode_baud_rate(&uart_term, 0, 38400);
    uart_term.c_cflag |= CS8;
    uart_term.c_oflag |= ONLCR;
    tty_termios_set(stddebug, &uart_term);
    tty_write(stddebug, (const uint8_t *)"UART is done\n", 13);
    tty_write(stddebug, (const uint8_t *)"UART is done\n", 13);

    vfs_init_done();
    device_debug();
    tty_write(stdout, (const uint8_t *)"VGA is done\n", 12);
    irq_arch_enable();

    trace(VFS_FD_STDOUT, LVL1, "Hello world from Kernel");
    log_info("MAIN", "main_boot_params @ %p", main_boot_params);
    log_debug("MAIN", "Hello world from Kernel");

    allocator_print_status();

    pci_init();
    pci_init_devices();

    device_debug();

    {
        fat_init();

        log_info("MAIN", "mounting drives");
        device_t *ahci;
        log_info("MAIN", "Getting drive 0x101");
        ahci = device_get_by_dev_id(MKDEV(DEVICE_BLOCK, 1, 2)); // device 1 partition 1
        log_info("MAIN", "Mounting drive %p", ahci);
        vfs_mount("/user!/", ahci, 0);
        log_info("MAIN", "Getting drive 0x100");
        ahci = device_get_by_dev_id(MKDEV(DEVICE_BLOCK, 1, 1)); // device 1 partition 0
        log_info("MAIN", "Mounting drive 0x100");
        vfs_mount("/boot!/boot", ahci, 0);

        Loader_init();
        ELF_init();
        syscall_init();
    }

    process_init();

    thread_t *t = thread_create_kernel(start_init, NULL);
    t->cpu_affinity = cpu_arch_get(1);
    sched_add(t);

    {
        while (main_thread->state != THREAD_DEAD)
        {
            // log_debug("M", "heartbeat");
            // sched_yield();
        }
        log_crit("MAIN", "main thread died");
        KERNEL_PANIC("MAIN", "main thread died");
    }
    // loop
    for (;;);
}

void kernel_main(boot_params_t *bootParams)
{
    // hexdump(bootParams, sizeof(boot_params_t));

    // log_debug(NO_MODULE, "from bootparams @ %p", bootParams);
    // log_debug(NO_MODULE, "kernel_address: %p", bootParams->kernel_address);
    // log_debug(NO_MODULE, "BootDevice: %x", bootParams->boot_device);
    // log_debug(NO_MODULE, "currentMode: %x", bootParams->current_mode);
    // log_debug(NO_MODULE, "e820Count: %x", bootParams->memory.count);
    // log_debug(NO_MODULE, "boot_flags: %x", bootParams->bootloader.boot_flags);
    // log_debug(NO_MODULE, "vesaModeCount: %x", bootParams->video.count);
    // log_debug(NO_MODULE, "rsdp_address: %p", bootParams->acpi.rsdp_address);

    ioremap_init();
    kstack_init();

    main_boot_params = setup_arch(bootParams);

    smp_arch_init(main_boot_params);

    timer_init(main_boot_params);

    // HALInit();

    log_debug("MAIN", "init main thread");
    allocator_print_blocks();
    main_thread = thread_create_main((vaddr_t)kernel_entry);
    thread_set_priority(main_thread, PRIORITY_HIGHEST);

    log_debug("MAIN", "new rsp = %p", main_thread->kernel_stack);
    // __asm__("mov rsp, %0" : : "r"((uint32_64)main_thread->kernel_stack));

    sched_init(main_thread);
    // log_debug("MAIN", "main thread stack @ %p", main_thread->kernel_stack);
    irq_arch_disable();
    schedule(NULL);
    while (true)
    {
        ;
    }

    __asm__("jmp kernel_entry");
    __builtin_unreachable();
}
