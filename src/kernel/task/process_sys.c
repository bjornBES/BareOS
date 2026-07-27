/*
 * File: process_sys.c
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process.h"
#include "task/threading/scheduling/scheduler.h"
#include "signal/signal.h"
#include <types.h>
#include "syscall/syscall.h"
#include "errno/errno.h"

#include "mm/vmm/vmm.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "VFS/path.h"

#define MODULE "PROC-SYS"

char *proc_get_cwd(char *buf, size_t size)
{
    thread_t *t = sched_get_current();
    ctx_dump(&t->ctx);
    if (buf == NULL)
    {
        SET_ERROR(EINVAL);
        return (void*)-EINVAL;
    }

    process_t *proc = process_get_current();
    if (vma_inside(proc->vma, (vaddr_t)buf) != RETURN_GOOD)
    {
        SET_ERROR(EFAULT);
        return (void*)-EFAULT;
    }

    size_t path_size = strlen(proc->path);
    char *path = proc->path;
    {
        const char *rest_segment = NULL;
        if (path_split_prefix(path, NULL, &rest_segment) != RETURN_GOOD)
        {
            log_crit(MODULE, "something is wrong with %p", path);
            return NULL;
        }
        const char *rest_segment_priv = NULL;
        while (path_next_segment(rest_segment, NULL, &rest_segment) == RETURN_GOOD)
        {
            if (rest_segment == NULL)
            {
                break;
            }
            rest_segment_priv = rest_segment;
        }
        path_size = (size_t)(rest_segment_priv - proc->path);
    }

    if (path_size > size)
    {
        SET_ERROR(ERANGE);
        return (void*)-ERANGE;
    }

    copy_to_user(buf, path, path_size);
    // log_debug(MODULE, "result = %s", buf);
    return buf;
}

SYSCALL_DEFINE2(proc_get_cwd, char *, size_t);

void proc_exit_group(int status)
{
    thread_t *current_thread = sched_get_current();
    process_t *proc = current_thread->proc;
    proc->exit_code.raw = status;

    process_unexec_process(proc);
    
    proc->parent->state = PROC_STATE_READY;
    signal_send_group(proc->parent, SIGCHLD);
    schedule(NULL);
}
SYSCALL_DEFINE1_NORETURN(proc_exit_group, int);