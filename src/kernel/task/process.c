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
#include "libs/malloc.h"

#include "kernel.h"

#include "arch/x86/gdt.h"

#include "VFS/vfs.h"
#include "debug/debug.h"
#include "allocator/stack_allocator.h"

#include <core/Defs.h>

#define MODULE "PROC"

void ASMCALL jump_to_user(void *user_entry, void *user_stack, pid process);

extern char stack_top;

process *current_process = NULL;
pid next_pid = 0;

int sys_execve(const char *path, char *argv[], char *envp[])
{
    return RETURN_ERROR;
}

process *process_create(char *path)
{
    frame_dump_bitmap();
    process *proc = malloc(sizeof(process));

    fd_t file = VFS_open(path);

    current_process = proc;
    log_debug(MODULE, "creating new user directory");
    paging_create_user_directory(proc);
    log_debug(MODULE, "at p%p, v%p", proc->page_dir_phys, proc->page_dir_virt);

    proc->regs.Reg32.esp = stack_alloc_init(proc);
    proc->regs.ss = x86_USER_DATA_SEGMENT;

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
    tss_entry.rsp0 = (uint64_t)&stack_top;
    log_debug(MODULE, "stack = %p esp = %p", proc->stack_top, proc->regs.Reg32.esp);
    log_debug(MODULE, "jumping to %p", proc->entry);

    uint64_t proc_pd_phys = (uint64_t)proc->page_dir_phys;
    log_debug(MODULE, "loading CR3 = %p", proc_pd_phys);

    // Switch to process page directory
    disableInterrupts();

    __asm__ volatile(
        "mov cr3, %0\n"
        ::"r"(proc_pd_phys) : "memory");
    enable_interrupts();

    uint8_t *entry = (void *)proc->entry;
    // log_debug(MODULE, "at virt address %p (%p)", entry, paging_get_physical(proc->page_dir_virt, (void *)entry));
    log_debug(MODULE, "bytes at entry: %02x %02x %02x %02x %02x %02x %02x %02x",
              entry[0], entry[1], entry[2], entry[3],
              entry[4], entry[5], entry[6], entry[7]);
    jump_to_user((void *)proc->regs.rip, (void *)proc->regs.rsp, proc->pid);
}

int exec(char *path, char *argv[])
{
    process *proc = process_create(path);

    uint32_t vaddr = proc->load_base;
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;

    uint32_t *pd = (uint32_t *)(uint64_t)proc->page_dir_virt;
    uint32_t pde = (uint32_t)(uint64_t)pd[pd_index];
    uint32_t *pt = (uint32_t *)(uint64_t)(pde & ~0xFFF);
    uint32_t pte = pt[pt_index];
    log_debug(MODULE, "PDE: %x  PTE: %x", pde, pte);

    process_run(proc);
    return RETURN_GOOD;
}

void process_init()
{
    next_pid = 0;
}
