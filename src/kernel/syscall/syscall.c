/*
 * File: syscall.c
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "syscall/syscall.h"
#include "asm/syscall_arch.h"

#include "pledge/pledge.h"

#include "thread/thread.h"
#include "process/process.h"

#include "debug/debug.h"

#include "init.h"

#include <syscall/syscall_uapi.h>

#include <config.h>

#define SYS_BARE    BIT(0)
#define SYS_POSIX   BIT(1)
#define SYS_FREEBSD BIT(2)
#define SYS_OPENBSD BIT(3)
#define SYS_LINUX   BIT(4)

#define CONFIG_ENABLED_SYS                    \
    ((CONFIG_USE_POSIX_SYS ? SYS_POSIX : 0) | \
     SYS_BARE)

#define sys_X(nr, name, sysname, sysnameupper, ...) extern status_t sys_##name(syscall_info_t *info, intptr_t *out_val);
#define X(nr, name, sysname, sysnameupper, ...) extern status_t name(syscall_info_t *info, intptr_t *out_val);
// #if (SYS_BARE & CONFIG_ENABLED_SYS) == SYS_BARE
#include <syscall/syscall_table_bareos.tbl>
// #endif
#undef sys_X
#undef X

#define CHECK_VERSION(_sys, version)                                                    \
    ((((_sys) & CONFIG_ENABLED_SYS) == SYS_BARE && (version) <= CONFIG_BAREOS_VERSION))

#define sys_X(nr, name, sysname, sysnameupper, ret, _pledge, _abi, _sys, version, ...) \
    [nr] = (syscall_function_info_t) {                                             \
        .number = nr,                                                              \
        .call_dose_return = ret,                                                   \
        .pledge = _pledge,                                                         \
        .abi = _abi,                                                               \
        .handler = (CHECK_VERSION(_sys, version))                                  \
            ? sys_##name                                                                 \
            : NULL},

#define X(nr, name, sysname, sysnameupper, ret, _pledge, _abi, _sys, version, ...) \
    [nr] = (syscall_function_info_t) {                                             \
        .number = nr,                                                              \
        .call_dose_return = ret,                                                   \
        .pledge = _pledge,                                                         \
        .abi = _abi,                                                               \
        .handler = (CHECK_VERSION(_sys, version))                                  \
            ? name                                                                 \
            : NULL},

syscall_function_info_t syscall_table[] = {
// #if (SYS_BARE & CONFIG_ENABLED_SYS) == SYS_BARE
#include <syscall/syscall_table_bareos.tbl>
    // #endif
};
#undef sys_X
#undef X

#define MODULE "SYSCALL"

status_t syscall_get_handler(uint32_t sys_number, syscall_function_info_t **out_info)
{
    // ENTER_FUNC("0x%x, %p", sys_number, out_info);
    if (sys_number >= SYSCALL_COUNT)
    {
        KERRNO_RETURN(KERRNO_BAD_INDEX, "");
    }
    syscall_function_info_t *syscall_info = (syscall_function_info_t *)&syscall_table[sys_number];
    if (syscall_info->handler)
    {
        *out_info = syscall_info;
        return KERRNO_SUCCESSES;
    }
    KERRNO_RETURN(KERRNO_BAD_VALUE, "");
}

status_t syscall_handler_func(syscall_info_t *info, syscall_function_info_t *func_info, intptr_t *out_ret)
{
    reg_t nr = info->sys_number;

    thread_t *current_thread = thread_get_current();

    if (!func_info->pledge && pledge_check(func_info->pledge))
    {
        process_t *current = current_thread->proc;
        KERRNO_RETURN(ENOSYS, "[%u]: pledge \"%s\", syscall %u", current->pid, pledge_get_missing(func_info->pledge), nr);
    }

    intptr_t ret_val = 0;
    status_t ret = 0;
    if (nr < SYSCALL_COUNT && func_info->handler)
    {
        ret = func_info->handler(info, &ret_val);
    }
    else
    {
        ret = ENOSYS;
    }
    // ivt_dump_frame(regs);
    // signal_try_deliver(current_t, info, regs);
    // log_debug(MODULE, "DONE");
    // sched_thread_info();
    *out_ret = ret_val;
    return ret;
}

extern boot_params_t *main_boot_params;

status_t syscall_init()
{
    ENTER_FUNC("", 0);
    return syscall_arch_init(main_boot_params);
}

SUBSYS_INITCALL(syscall_init);
