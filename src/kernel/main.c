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

#include "dev/device.h"
#include "timer/timer.h"

#include "smp/smp.h"

#include "debug/debug.h"

#include "pci/pci.h"

#include "vfs/vfs.h"

#include "thread/thread.h"
#include "sched/sched.h"

#include <boot/params.h>

#define MODULE "main"

boot_params_t *main_boot_params;

extern void hexdump(void *ptr, size_t len, size_t size);

NORETURN void kernel_main()
{
    ENTER_FUNC("", 0);

    trace_info(NO_MODULE, "timer_now_ticks() = %lld", timer_now_ticks());
    trace_info(NO_MODULE, "timer_now_ns()    = %lld", timer_now_ns());

    pci_initialize();

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_PREUSER);

    device_debug();

    vfs_node_t *node;
    vfs_open("/@BOOT/dir/sub/file.txt", 0, 0, &node);
    hexdump(node->inode, sizeof(inode_t), 16);
    trace_debug(MODULE, "node = %p", node);
    char buf1[512];
    ssize_t bytes_read1 = 0;
    status_t ret1 = vfs_read(node, buf1, 512, &bytes_read1);
    // hexdump(node, sizeof(vfs_node_t), 16);
    hexdump(node->inode, sizeof(inode_t), 16);
    vfs_node_t *user_node;
    vfs_open("/@BOOT/dir/sub/file.txt", 0, 0, &user_node);
    // hexdump(node, sizeof(vfs_node_t), 16);
    // hexdump(user_node, sizeof(vfs_node_t), 16);

    hexdump(node->inode, sizeof(inode_t), 16);
    hexdump(user_node->inode, sizeof(inode_t), 16);
    vfs_close(node);
    hexdump(user_node->inode, sizeof(inode_t), 16);
    
    vfs_open("/@BOOT/dir/sub/file.txt", 0, 0, &node);
    ret1 = vfs_read(node, buf1, 512, &bytes_read1);
    hexdump(user_node->inode, sizeof(inode_t), 16);
    
    char buf2[512];
    ssize_t bytes_read2 = 0;
    status_t ret2 = vfs_read(user_node, buf2, 512, &bytes_read2);
    hexdump(user_node->inode, sizeof(inode_t), 16);
    vfs_close(user_node);
    hexdump(node->inode, sizeof(inode_t), 16);
    vfs_close(node);

    if (ret1 == KERRNO_SUCCESSES)
    {
        buf1[bytes_read1] = 0;
        trace_debug(MODULE, "buf = \"%s\"", buf1);
        // hexdump(buf, bytes_read, 16);
    }
    if (ret2 == KERRNO_SUCCESSES)
    {
        buf2[bytes_read2] = 0;
        trace_debug(MODULE, "buf = \"%s\"", buf2);
        // hexdump(buf, bytes_read, 16);
    }

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
