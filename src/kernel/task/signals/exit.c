/*
 * File: exit.c
 * File Created: 15 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "exit.h"

#include "kernel.h"

#include "task/process.h"

int exit_exit(uint32_t code)
{
    return do_exit(code, current_process);
}
SYSCALL_DEFINE1(exit_exit, int, uint32_t)

int do_exit(uint32_t code, process *proc)
{
    log_debug("EXIT", "exit from %u with code %u", proc->pid, code);
    if (proc->pid == 0)
    {
        kernel_panic("Attempted to kill init! exitcode=0x%08x", code);
        return RETURN_ERROR;
    }

    process_unexec_process(proc->pid);

    return RETURN_FAILED;
}