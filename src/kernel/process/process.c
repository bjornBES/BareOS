/*
 * File: process.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process/processt.h"
#include "process/process.h"
#include "process/pid.h"

#include "sched/sched.h"

#include "sync/spinlock.h"

#include "memory.h"

#define MODULE "process"

ALWAYS_INLINE INTERNAL void process_lock(process_t *proc)
{
    spinlock_acquire(&proc->pdb_lock);
}

ALWAYS_INLINE INTERNAL void process_unlock(process_t *proc)
{
    spinlock_release(&proc->pdb_lock);
}

#define LOCK_NAME proc_lock
DEF_LOCK(LOCK_NAME);

status_t proc_allocate(process_t *parent, process_t **out_proc)
{
    ENTER_FUNC("%p, %p", parent, out_proc);
    process_t *proc = kzmalloc(sizeof(process_t));
    *out_proc = proc;
    if (parent)
    {
        proc->parent = parent;
        proc->pid_blk = parent->pid_blk;
    }
    status_t ret = pid_allocate(proc);
    KERRNO_CHECK_RETURN(ret);
    list_initialize(&proc->children);
    list_initialize(&proc->threads);
    return ret;
    // sched_add(main_thread);
}

status_t proc_add_thread(process_t *proc, thread_t *t)
{
    t->proc = proc;
    sched_add(t);
    if (!proc->main_thread && t->is_main_thread)
    {
        proc->main_thread = t;
    }
    return ADD_THIS_TO_LIST(proc->threads, t);
}

status_t proc_add(process_t *parent, process_t *child)
{
    ADD_THIS_TO_LIST(parent->children, child);
    child->parent = parent;
    return KERRNO_SUCCESSES;
}

status_t proc_run(process_t *proc)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESSES;
}

