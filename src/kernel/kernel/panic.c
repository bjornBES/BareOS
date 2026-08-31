/*
 * File: panic.c
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "panic.h"

#include "debug/debug.h"

#include <types.h>

void kernel_panic(const char *sub_sys, char *file, int line, const char *function, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    log_crit(NO_MODULE, "KERNEL PANIC");
    log_crit(NO_MODULE, "%s:%i in function %s", file, line, function);
    logfl_args(sub_sys, LVL_INFO, fmt, args);

    va_end(args);

    // TODO: make it reboot the system
    for (;;)
    {
        ;
    }
    
}
