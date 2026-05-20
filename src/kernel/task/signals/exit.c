/*
 * File: exit.c
 * File Created: 15 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "exit.h"

#include "kernel.h"

#include "task/threading/scheduling/scheduler.h"
#include "task/process.h"

int exit_exit(uint32_t code)
{
    do_exit(code, process_get_current());
    return 0;
}
SYSCALL_DEFINE1(exit_exit, uint32_t)

void do_exit(uint32_t code, process_t *proc)
{
    log_debug("EXIT", "exit from %u with code %u", proc->pid, code);
    if (proc->pid == 0)
    {
        kernel_panic("Attempted to kill init! exitcode=0x%08x", code);
    }

    proc->exit_code.exit_code1 = code;
    proc->state = PROC_STATE_ZOMBIE;

    if (proc->parent)
    {
        thread_t *parent_thread = scheduler_find_waiting(proc->parent);
        if (parent_thread && proc->parent->wait_for == proc->pid)
        {
            scheduler_unblock(parent_thread);
        }
    }

    process_unexec_process(proc);

    schedule(NULL);
}