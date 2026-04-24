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

#include "signals/exit.h"
#include "signal.h"

#include <core/Defs.h>

#define MODULE "PROC"

extern char stack_top;

process *current_process = NULL;
pid next_pid = 0;

void process_user_page_fault(Registers *regs)
{
    fprintf(VFS_FD_DEBUG, "[Page Fault] User process\n");
    virt_addr cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));
    paging_print_info(cr2);
    char buf[2048];
    x86_ISRFormatRegisters(regs, buf, "\n", "[Page Fault]   ");

    fprintf(VFS_FD_DEBUG, "[Page Fault] Virtual address %p\n", cr2);

    fprintf(VFS_FD_DEBUG, "[Page Fault] Unhandled Page Fault %d\n", regs->interrupt);
    fprintf(VFS_FD_DEBUG, "%s", buf);

    fprintf(VFS_FD_DEBUG, "[Page Fault] Error code 0x%x\n", regs->error);
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Present %u\n", BIT_GET(regs->error, 0));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Write %u\n", BIT_GET(regs->error, 1));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    User %u\n", BIT_GET(regs->error, 2));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Reserved write %u\n", BIT_GET(regs->error, 3));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Instruction Fetch %u\n", BIT_GET(regs->error, 4));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Protection key %u\n", BIT_GET(regs->error, 5));

    KernelPanic("Page Fault", "Got page fault from %x", cr2);
}

int sys_execve(const char *path, char *argv[], char *envp[])
{
    return RETURN_ERROR;
}

process *process_create(char *path)
{
    // frame_dump_bitmap();
    process *proc = malloc(sizeof(process));

    memset(proc, 0, sizeof(process));

    fd_t file = VFS_open(path);

    log_debug(MODULE, "creating new user directory");
    paging_create_user_directory(proc);
    log_debug(MODULE, "at p%p, v%p", proc->page_dir_phys, proc->page_dir_virt);

    proc->regs.sp = (uint32_64)stack_alloc_init(proc);
    proc->regs.ss = x86_USER_DATA_SEGMENT;

    loader *loader = Loader_probe(file);
    int state = loader->load(file, proc, loader);
    VFS_close(file);
    if (state != RETURN_GOOD)
    {
        free(proc);
        return NULL;
    }
    proc->regs.pc = (uint32_64)proc->entry;

    proc->pid = next_pid;
    next_pid++;

    proc->ladder.next = (task_ladder *)current_process;

    return proc;
}

extern void hexdump(void *ptr, int len);

void process_run(process *proc)
{
    tss_entry.sp0 = (uint32_64)&stack_top;
    log_debug(MODULE, "USER: stack = %p sp = %p", proc->stack_top, proc->regs.sp);
    log_debug(MODULE, "TSS: sp = %p", tss_entry.sp0);
    log_debug(MODULE, "jumping to %p", proc->entry);

    phys_addr proc_pd_phys = (phys_addr)((uint32_64)proc->page_dir_phys.page_dir);
    log_debug(MODULE, "proc->page_dir_phys = %p", proc->page_dir_phys);
    log_debug(MODULE, "proc->page_dir_phys->page_dir = %p", proc->page_dir_phys.page_dir);
    log_debug(MODULE, "loading CR3 = %p", proc_pd_phys);

    // Switch to process page directory
    disableInterrupts();
    __asm__ volatile(
        "mov cr3, %0\n" ::"r"((uint64_t)proc_pd_phys) : "memory");
    enable_interrupts();

    uint8_t *entry = (uint8_t *)proc->entry;
    log_debug(MODULE, "at virt address %p (%p)", entry, paging_get_physical(proc->page_dir_virt, (void *)entry));
    hexdump(entry, 16);

#define inline_asm __asm__ __volatile__
    inline_asm(
        "mov ax, 0x23\n\t"
        "mov ds, ax\n\t"
        "mov es, ax\n\t"
        "mov fs, ax\n\t"
        "mov gs, ax\n\t"
        "push 0x23\n\t"
        "push %[proc_stack]\n\t"
        "pushf\n\t"
#ifdef __x86_64__
        "pop rdi\n\t"
        "or rdi, 0x200\n\t"
        "push rdi\n\t"
#else
        "pop edi\n\t"
        "or edi, 0x200\n\t"
        "push edi\n\t"
#endif
        "push 0x1B\n\t"
        "push %[proc_entry]\n\t"
#ifdef __x86_64__
        "iretq\n\t"
#else
        "iret\n\t"
#endif
        : : [proc_pid] "r"(proc->pid),
        [proc_stack] "r"(proc->stack_top), [proc_entry] "r"(proc->entry), [res] "D"(0) :);
// jump_to_user((void *)proc->regs.pc, (void *)proc->regs.sp, proc->pid);
#undef inline_asm
}

void process_unexec_process(pid pid)
{
}

int process_exec(char *path, char *argv[])
{
    process *proc = process_create(path);
    if (current_process != NULL)
    {
        if (!current_process->child)
        {
            current_process->child = proc;
        }
        else
        {
            task_ladder *current = current_process->ladder.next;
            for (size_t i = 0; i < current_process->ladder_count; i++)
            {
                if (!current->next)
                {
                    log_err(MODULE, "something fucked up i think pid=%u base=%u", ((process *)current)->pid, current_process->pid);
                    break;
                }
                current->next->priv = current;
                current = current->next;
            }

            current->next = (task_ladder *)proc;
        }
    }
    current_process = proc;
    process_run(proc);
    return RETURN_GOOD;
}

int process_kill(pid pid, int sig)
{
    log_debug(MODULE, "killing procress %u with %u", pid, sig);
    // TODO: switch the current process
    process *proc = current_process;
    if (sig == SIGKILL)
    {
        return do_exit(137, proc);
    }

    return RETURN_FAILED;
}

void process_init()
{
    x86_isr_register_handler(14, process_user_page_fault);

    log_debug(MODULE, "init process");
    next_pid = 0;
}
