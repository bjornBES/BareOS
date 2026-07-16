/*
 * File: syscall.c
 * File Created: 22 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "syscall.h"
#include <syscall/syscall_number.h>

#include "debug/debug.h"
#include "errno/errno.h"
#include "task/threading/scheduling/scheduler.h"
#include "task/process.h"
#include "signal/signal.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/ivt.h"

#define X(nr, name, sysname, sysnameupper, ...) extern uintptr_t sys_##name(syscall_info *info, syscall_function_info *sys_info);
#include <syscall/syscall_table.tbl>
#undef X

#define X(nr, name, sysname, sysnameupper, ret, pledge, abi, ...) [nr] = {nr, sys_##name, ret, pledge, abi},
static const syscall_function_info syscall_table[] = {
#include <syscall/syscall_table.tbl>
};
#undef X

#define MODULE "SYSCALL"

int syscall_get_handler(uint32_t sys_number, syscall_function_info *out_info)
{
    if (sys_number >= SYSCALL_COUNT)
    {
        return RETURN_FAILED;
    }
    syscall_function_info syscall_info = syscall_table[sys_number];
    if (syscall_info.handler)
    {
        memcpy(out_info, &syscall_table[sys_number], sizeof(syscall_function_info));
        return RETURN_GOOD;
    }
    return RETURN_FAILED;
}

uintptr_t syscall_handler_func(syscall_info *info, syscall_function_info *func_info, intr_frame_t *regs)
{
    reg_t nr = info->sys_number;
    syscall_function_info syscall_info = syscall_table[nr];

    thread_t *current_t = scheduler_get_current();

    if (syscall_info.pledge != 0 && pledge_check(syscall_info.pledge) != RETURN_GOOD)
    {
        process_t *current = process_get_current();
        fprintf(VFS_FD_DEBUG, "[%u]: pledge \"%s\", syscall %u,\n", current->pid, pledge_get_missing(syscall_info.pledge), nr);
        signal_send_group(current, SIGSYS);
        signal_try_deliver(current_t, info, regs);
        return -ENOSYS;
    }

    uintptr_t ret = 0;
    if (nr < SYSCALL_COUNT && syscall_info.handler)
    {
        ret = syscall_info.handler(info, &syscall_info);
        if (syscall_info.call_dose_return)
        {
        }
    }
    else
    {
        ret = -ENOSYS;
    }
    // ivt_dump_frame(regs);
    signal_try_deliver(current_t, info, regs);
    return ret;
}

void syscall_init()
{
}
