/*
 * File: user_sys.c
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */


#include "syscall/syscall.h"
#include <types.h>

#include "task/process.h"

uid_t get_user_id()
{
    process_t *proc = process_get_current();
    return proc->user;
}
SYSCALL_DEFINE0(get_user_id);

uid_t get_eff_user_id()
{
    process_t *proc = process_get_current();
    return (proc->user << 8) + proc->user;
}
SYSCALL_DEFINE0(get_eff_user_id);