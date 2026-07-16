/*
 * File: sys_thread.c
 * File Created: 07 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "thread_type.h"
#include "task/threading/scheduling/scheduler.h"
#include "syscall/syscall.h"

tid_t set_tid_address(int *tidptr)
{
    thread_t *current = scheduler_get_current();
    current->clear_child_tid = tidptr;
    log_debug(NO_MODULE, "returning with 0x%x", current->tid);
    return current->tid;
}

SYSCALL_DEFINE1(set_tid_address, int *);