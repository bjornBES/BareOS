/*
 * File: kernel.c
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel.h"
#include "kernel/irq.h"
#include "kernel/memory.h"
#include "debug/debug.h"
#include "stdio.h"
#include <defs.h>
#include "kernel/io.h"

#include <stdint.h>
#include <printf_driver/printf.h>

void panic(char *module, char *file, int line, char* message, ...)
{
    irq_arch_disable();
    va_list args;
    va_start(args, message);
    // printf("[KERNEL] Kernel Panic\n");
    // printf("[KERNEL] In file %s:%u\n", file, line);
    // vprintf(VFS_FD_STDERR, message, args);
    fprintf(VFS_FD_DEBUG, "[KERNEL] Kernel Panic\n");
    fprintf(VFS_FD_DEBUG, "[KERNEL] In file %s:%u\n", file, line);
    vprintf(VFS_FD_DEBUG, message, args);               // write text
    fputs("\n", VFS_FD_DEBUG);                          // newline
    va_end(args);
    // irq_arch_mask(0x22);
    irq_arch_disable();
    inline_asm("hlt" : : : "memory");
    for (;;)
    ;
}
void kernel_panic(char* message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(VFS_FD_DEBUG, "[KERNEL] Kernel Panic\n");
    vprintf(VFS_FD_DEBUG, message, args);               // write text
    fputs("\n", VFS_FD_DEBUG);                          // newline
    va_end(args);
    // TODO make PC restart
    irq_arch_disable();
    inline_asm("hlt" : : : "memory");
    for (;;)
    ;
}
