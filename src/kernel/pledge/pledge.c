/*
 * File: pledge.c
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pledge/pledge.h"

#include "process/process.h"

#include "kerrno.h"

status_t pledge_check(pledge_flags_t function_pledge)
{
    process_t *current_process;
    process_get_current(&current_process);
    if (!current_process->pledged)
    {
        return KERRNO_UNSUCCESS;
    }
    if ((current_process->pledge_mask & function_pledge) == function_pledge)
    {
        return KERRNO_SUCCESSES;
    }

    return KERRNO_UNSUCCESS;
}

char *pledge_get_missing(pledge_flags_t function_pledge)
{
    pledge_flags_t pledge = function_pledge;
    switch (pledge)
    {
        case PLEDGE_STDIO :
            return "STDIO";
        case PLEDGE_RPATH :
            return "RPATH";
        case PLEDGE_PROC :
            return "PROC";
        case PLEDGE_MALLOC :
            return "MALLOC";

        default :
            break;
    }
    return NULL;
}