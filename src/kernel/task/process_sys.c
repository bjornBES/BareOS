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
