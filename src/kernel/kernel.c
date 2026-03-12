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

#include <stdint.h>
#include <stdio.h>
#include <IO.h>
#include <core/printfDriver/printf.h>

void panic(char *module, char *file, int line, char* message, ...)
{
    va_list args;
    va_start(args, message);
    printf("[KERNEL] Kernel Panic\n");
    printf("[KERNEL] In file %s:%u\n", file, line);
    vprintf(VFS_FD_STDERR, message, args);
    log_crit("KERNEL", "Kernel Panic");
    log_err("KERNEL", "In file %s:%u", file, line);
    fputs("\033[1;31m", VFS_FD_DEBUG);                  // set color depending on level
    fprintf(VFS_FD_DEBUG, "[%s] ", module);             // write module
    vprintf(VFS_FD_DEBUG, message, args);               // write text
    fputs("\033[0m", VFS_FD_DEBUG);                     // reset format
    fputs("\n", VFS_FD_DEBUG);                          // newline
    va_end(args);
    disableInterrupts();
    for (;;)
    ;
}