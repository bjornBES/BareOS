/*
 * File: proc_wait.c
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process.h"
#include "process_internal.h"
#include "syscall/syscall.h"
#include "task/threading/scheduling/scheduler.h"

#include "errno/errno.h"

#define MODULE "PROCESS_WAIT"

#define WNOHANG    1
#define WUNTRACED  2

#define WSTOPPED   2
#define WEXITED    4
#define WCONTINUED 8
#define WNOWAIT    0x1000000

pid_t process_waitpid(pid_t child_pid, int *wstatus, int options)
{
    process_t *parent = process_get_current();

    process_t *child;
    if (child_pid > 0)
    {
        child = process_find_child(parent, child_pid);
    }
    else
    {
        ERRNO_RETURN(ENOSYS, "function part not implemented", 0);
    }
    if (child == NULL)
    {
        process_list(parent);
        ERRNO_RETURN(ECHILD, "The process specified by pid (%u) does not exist or is not a child of the calling process.", child_pid);
    }

    while (true)
    {
        if (child->state != PROC_STATE_ZOMBIE)
        {
            if (FLAG_IS_SET(options, WNOHANG))
            {
                return 0;
            }
            parent->wait_for = child_pid;
            sched_block(&parent->children_wait);
        }
        else
        {
            break;
        }
    }
    parent->wait_for = 0;

    if (wstatus)
    {
        *wstatus = child->exit_code.raw;

        // [31-24] RES
        // [23-16] exitcode
        // [15-8] exitcode
        // [7-0] signal
    }

    // process_reap(child);
    return child_pid;
}

SYSCALL_DEFINE3(process_waitpid, pid_t, int *, int);