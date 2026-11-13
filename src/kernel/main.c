/*
 * File: main.c
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "init.h"

#include "acpi/fadt/fadt.h"

#include "resource/resource.h"

// #include "drivers/video/console/fbcon.h"
#include "drivers/tty/vt/vt.h"
#include "drivers/tty/tty.h"

#include "dev/device.h"
#include "timer/timer.h"

#include "smp/smp.h"

#include "debug/debug.h"

#include "drivers/pci/pci.h"

#include "vfs/vfs.h"

#include "thread/thread.h"
#include "sched/sched.h"

#include "drivers/platform/platform.h"

#include "memory.h"

#include <boot/params.h>

#define MODULE "main"

boot_params_t *main_boot_params;

extern void hexdump(void *ptr, size_t len, size_t size);

extern void kernel_start_pid0();

NORETURN void kernel_main()
{
    ENTER_FUNC("", 0);

    trace_info(NO_MODULE, "timer_now_ticks() = %lld", timer_now_ticks());
    trace_info(NO_MODULE, "timer_now_ns()    = %lld", timer_now_ns());

    pci_initialize();

    platform_device_t *platform_dev = kzmalloc(sizeof(platform_device_t));
    platform_dev->name = "i8042";
    platform_device_register(platform_dev);

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_PREUSER);

    device_debug();

    resource_dump(&mem_space.root);
    resource_dump(&io_space.root);
    resource_dump(&irq_space.root);

    tty_write(active_tty, (uint8_t *)"Hello world!\n\n", 14, NULL);
    tty_write(active_tty, (uint8_t *)"Hello world!", 12, NULL);

    uint8_t input[128];
    ssize_t size;
    // tty_read(active_tty, input, 128, &size);
    // hexdump(input, size, 16);
    // input[size] = 0;
    // trace_info(MODULE, "size = %u", size);
    // trace_info(MODULE, "input = %s", input);

    // while (true)
    // {
    //     ;
    // }

    kernel_start_pid0();

    for (;;)
    {
        ;
    }
}

THREAD_WARPER_NO_RETURN_ARG(kernel_main);

__init void kernel_early_main(boot_params_t *boot_params)
{
    main_boot_params = boot_params;

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_ARCHDONE);

    smp_init(main_boot_params);

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_SUBSYS);

    vfs_initialize();

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_ROOTFS);

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_DEVICE);

    /*     while (true)
        {
        } */

    fadt_parse();

    trace_info(NO_MODULE, "timer_now_ticks() = %lld", timer_now_ticks());
    trace_info(NO_MODULE, "timer_now_ns()    = %lld", timer_now_ns());

    device_debug();

    thread_t *main_thread = thread_create_kernel(THREAD_CALL_FUNC(kernel_main), 0, 0);
    sched_init(main_thread);

    trace_info(MODULE, "jump to kernel_main");

    schedule(NULL);

    for (;;)
    {
        ;
    }
}
