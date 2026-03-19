/*
 * File: process.c
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process.h"
#include "loader.h"
#include "malloc.h"

#include "kernel.h"

#include "arch/i686/gdt.h"

#include "VFS/vfs.h"
#include "debug/debug.h"
#include "allocator/stack_allocator.h"

#include <core/Defs.h>

#define MODULE "PROC"

void ASMCALL jump_to_user(void *user_entry, void *user_stack);

extern char stack_top;

process *current_process = NULL;
pid next_pid = 0;

int sys_execve(const char *path, char *argv[], char *envp[])
{
    return RETURN_ERROR;
}

process *process_create(char *path)
{
    process *proc = malloc(sizeof(process));
    
    fd_t file = VFS_open(path);

    current_process = proc;
    proc->page_dir = paging_create_user_directory();

    proc->regs.Reg32.esp = stack_alloc_init(proc);
    proc->regs.ss = i686_USER_DATA_SEGMENT;

    loader *loader = Loader_probe(file);
    int state = loader->load(file, proc, loader);
    VFS_close(file);
    if (state != RETURN_GOOD)
    {
        free(proc);
        return NULL;
    }
    proc->regs.Reg32.eip = proc->entry;

    proc->pid = next_pid;
    next_pid++;

    return proc;
}

void process_run(process *proc)
{
    tss_entry.esp0 = (uint32_t)&stack_top;
    log_debug(MODULE, "stack = %p esp = %p", proc->stack_top, proc->regs.Reg32.esp);
    log_debug(MODULE, "jumping to %p", proc->entry);
    jump_to_user((void*)proc->regs.Reg32.eip, (void*)proc->regs.Reg32.esp);
}

int exec(char *path, char *argv[])
{
    process *proc = process_create(path);
    process_run(proc);
    return RETURN_GOOD;
}
