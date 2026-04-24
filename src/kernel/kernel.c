/*
 * File: kernel.c
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel.h"
#include "debug/debug.h"
#include "libs/stdio.h"
#include "libs/IO.h"

#include <stdint.h>
#include <printf_driver/printf.h>

void panic(char *module, char *file, int line, char* message, ...)
{
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
    disableInterrupts();
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
    disableInterrupts();
    for (;;)
    ;
}