/*
 * File: pledge.c
 * File Created: 15 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pledge.h"

#include "kernel.h"
#include "syscall.h"
#include "task/process.h"

#include "libs/string.h"

int pledge_pledge(pledge_flags_t requested) {
    if (current_process->pledged)
    {
        requested &= current_process->pledges; // can only restrict further
    }
    log_debug("PLEDGE", "process %u requesting %08b", current_process->pid, requested);
    current_process->pledges = requested;
    current_process->pledged = true;
    return RETURN_GOOD;
}
SYSCALL_DEFINE1(pledge_pledge, int, pledge_flags_t)

int pledge_check(pledge_flags_t function_pledge)
{
    if (!current_process->pledged)
    {
        return RETURN_FAILED;
    }
    if ((current_process->pledges & function_pledge) == function_pledge)
    {
        return RETURN_GOOD;
    }

    return RETURN_FAILED;
}
char *pledge_get_missing(pledge_flags_t function_pledge)
{
    pledge_flags_t pledge = function_pledge;
    switch (pledge)
    {
    case PLEDGE_STDIO:
        return "STDIO";
        case PLEDGE_RPATH:
        return "RPATH";
    
    default:
        break;
    }
    return NULL;
}
