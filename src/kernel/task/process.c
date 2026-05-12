/*
 * File: process.c
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 23:50:11
 * Modified By: BjornBEs
 * -----
 */

#include "process.h"
#include "loader.h"
#include "libs/malloc.h"
#include "libs/string.h"
#include "libs/stdio.h"

#include "memory/pmm/pmm.h"
#include "memory/vmm/vmm.h"
#include "memory/memdefs.h"
#include "threading/thread.h"
#include "threading/scheduling/scheduler.h"

#include "kernel/gdt.h"
#include "kernel/task/tss.h"
#include "kernel.h"

#include "VFS/vfs.h"
#include "debug/debug.h"

#include "signals/exit.h"
#include "signal.h"

#include <defs.h>

#define MODULE "PROC"

#define MAX_PROCESSES 64
#define MAX_ARGS 64
process_t *pid_table[MAX_PROCESSES];

extern char stack_top;

process_t *current_process = NULL;
pid_t next_pid = 0;

void process_user_page_fault(paging_info *info)
{
    fprintf(VFS_FD_DEBUG, "[Page Fault] User process\n");
    paging_print_info(info->page_directory, info->fault_addr);

    if (info->pc >= (uint64_t)KERNEL_VIRT_BASE)
    {
        schedule(NULL);
    }

    if (info->as_kernel)
    {
        fprintf(VFS_FD_DEBUG, "[Page Fault] active is kernels (%p)\n", kernel_page);
    }
    log_info(MODULE, "came from %p", info->page_directory.page_dir);

    thread *current_thread = scheduler_get_current();
    log_info(MODULE, "comes from thread %u", current_thread->tid);
    if (current_thread->state == THREAD_DEAD)
    {
        KernelPanic("Page Fault", "Got page fault from %p", info->fault_addr);
    }
    if (current_thread->tid != 0 && current_thread->state != THREAD_DEAD)
    {
        uint32_64 func = current_thread->ctx.regs.pc;
        log_info(MODULE, "thread function %x", func);
        scheduler_thread_exit();
        return;
    }

    pmm_print_info_verbose();

    fprintf(VFS_FD_DEBUG, "[Page Fault] Virtual address %p\n", info->fault_addr);
    fprintf(VFS_FD_DEBUG, "[Page Fault] Physical address %p\n", paging_get_physical(kernel_page, (virt_addr)info->fault_addr));

    if (info->as_kernel || current_process == NULL)
    {
        KernelPanic("Page Fault", "Got page fault from %p", info->fault_addr);
    }
    KernelPanic("Page Fault", "Got page fault from %p", info->fault_addr);
    // process_kill(current_process->pid, SIGSEGV);
}

extern void hexdump(void *ptr, int len);
process_t *process_create()
{
    process_t *proc = malloc(sizeof(process_t));
    memset(proc, 0, sizeof(process_t));

    log_info(MODULE, "creating new user directory");
    paging_create_user_directory(proc);
    paging_load_cr3(proc->page_dir);
    log_info(MODULE, "user directory at v%p/p%p", proc->page_dir.page_dir, proc->page_dir.page_dir_phys);

    vma_init(proc);

    log_info(MODULE, "mmap stack proc->vma = %p", proc->vma);
    vma_t *stack = do_mmap(proc->vma, USER_STACK_TOP - PAGE_SIZE, PAGE_SIZE, VMA_ANONYMOUS, stack_flags);
    log_info(MODULE, "mmap heap proc->vma = %p", proc->vma);
    vma_t *heap = do_mmap(proc->vma, USER_HEAP_BASE, PAGE_SIZE, VMA_ANONYMOUS, data_flags);

    proc->vma->start_stack = stack->end;
    proc->vma->start_heap = heap->start;
    proc->vma->end_heap = heap->end;

    log_info(MODULE, "mapping signals mapping %p to %p", KERNEL_VDSO_PHYS, USER_VDSO_VIRT);
    // kernel side, when setting up a new process
    paging_map_page(proc->page_dir, USER_VDSO_VIRT, KERNEL_VDSO_PHYS, vdso_text_flags);

    proc->pid = next_pid;
    pid_table[next_pid] = proc;
    next_pid = next_pid + 1;
    return proc;
}
int process_load(char *path, process_t *proc)
{
    proc->volume = strcpy(proc->volume, path);

    fd_t file = fopen(path, "");
    loader *loader = Loader_probe(file);
    log_info(MODULE, "loading proc->vma = %p", proc->vma);
    int state = loader->load(file, proc, loader);
    if (state != RETURN_GOOD)
    {
        fclose(file);
        free(proc);
        return RETURN_FAILED;
    }
    fclose(file);
    return RETURN_GOOD;
}

pid_t process_fork(syscall_info *info)
{
    // 1.
    process_t *parent = process_get_current();
    process_t *child = kmalloc(sizeof(process_t));
    memcpy(child, parent, sizeof(process_t));

    memset(&child->ladder, 0, sizeof(task_ladder_t));
    child->parent = parent;
    child->child = NULL;

    child->thread_count = 0;
    memset(child->threads, 0, sizeof(thread *) * MAX_THREADS_PER_PROCESS);

    // 2.
    log_info(MODULE, "creating new user directory");
    paging_create_user_directory(child);
    child->vma = vma_clone(parent->vma, child);
    child->vma->page_directory = child->page_dir;
    paging_copy_kernel_mappings(child);
    log_info(MODULE, "done cloning");

    // 3.
    log_info(MODULE, "walk process tree");
    if (!parent->child)
    {
        // first child
        parent->child = child;
    }
    else
    {
        // walk sibling chain to tail
        task_ladder_t *current = (task_ladder_t *)parent->child;
        while (current->next)
        {
            current->next->priv = current; // fix: set priv on next before moving
            current = current->next;
        }
        // append and set back-pointer
        current->next = (task_ladder_t *)child;
        ((task_ladder_t *)child)->priv = current;
        child->parent = parent;
    }
    log_info(MODULE, "DONE walk process tree");

    // 4.
    log_info(MODULE, "signals");
    memcpy(&child->signal_table, &parent->signal_table, sizeof(signal_table));
    memset(&child->signal_queue, 0, sizeof(signal_pending));
    paging_map_page(child->page_dir, USER_VDSO_VIRT, KERNEL_VDSO_PHYS, vdso_text_flags);
    log_info(MODULE, "DONE signals");

    // 5, 6, N.
    thread *parent_thread = scheduler_get_current();
    thread *child_thread = thread_create_from(parent_thread);
    registers *child_regs = (registers *)child_thread->ctx.stack_pointer;
    log_info(MODULE, "t%u entry = %p", child_thread->tid, child_regs->pc);
    child_thread->proc = child;
    child_regs->result = 0; // same as ax on x86

    // 8.
    child->threads[0] = child_thread;
    child->thread_count++;
    child->state = PROC_STATE_RUNNING;
    scheduler_add(child_thread);

    child->pid = next_pid;
    pid_table[next_pid] = child;
    next_pid = next_pid + 1;

    // 9.
    log_debug(MODULE, "forked %u from %u", child->pid, parent->pid);
    return next_pid - 1;
}
SYSCALL_DEFINE0_REG(process_fork);

void process_run(process_t *proc)
{
    thread *curr = scheduler_get_current();
    log_debug(MODULE, "USER: stack = %p", proc->vma->start_stack);
    log_debug(MODULE, "TSS: sp = %p", tss_entry.sp0);
    log_debug(MODULE, "jumping to %p", proc->entry);

    paging_copy_kernel_mappings(proc);

    scheduler_add(proc->threads[proc->thread_count - 1]);
    return;
}

void process_reap(process_t *proc)
{
    // reap all children first
    process_t *child = proc->child;
    while (child)
    {
        process_t *next = (process_t *)child->ladder.next;
        process_reap(child);
        child = next;
    }

    // remove all threads from scheduler
    bool need_reschedule = false;
    for (size_t i = 0; i < MAX_THREADS_PER_PROCESS; i++)
    {
        if (proc->threads[i] != NULL)
        {
            if (proc->threads[i]->state == THREAD_RUNNING)
            {
                need_reschedule = true;
            }
            scheduler_remove(proc->threads[i]);
            proc->threads[i] = NULL;
        }
    }

    // unlink from parent
    if (proc->parent && proc->parent->child == proc)
    {
        proc->parent->child = (process_t *)proc->ladder.next;
    }
    if (proc->ladder.priv)
    {
        proc->ladder.priv->next = proc->ladder.next;
    }
    if (proc->ladder.next)
    {
        ((task_ladder_t *)proc->ladder.next)->priv = proc->ladder.priv;
    }

    // free resources
    pid_table[proc->pid] = NULL;
    vma_free(proc);
    paging_free_user_directory(proc);
    paging_unmap_page(proc->page_dir, USER_VDSO_VIRT);
    free(proc);

    if (need_reschedule)
    {
        schedule(NULL);
    }
}

int process_unexec_process(process_t *proc)
{
    proc->state = PROC_STATE_ZOMBIE;

    for (size_t i = 0; i < MAX_THREADS_PER_PROCESS; i++)
    {
        if (proc->threads[i] != NULL)
        {
            proc->threads[i]->state = THREAD_REMAINS;
            proc->threads[i] = NULL;
        }
    }

    return RETURN_GOOD;
}

void waitpid_loop()
{
    while (true)
    {
        process_t *child = current_process->child;
        while (child)
        {
            process_t *next = (process_t *)child->ladder.next;
            if (child->state == PROC_STATE_ZOMBIE)
            {
                process_reap(child);
            }
            child = next;
        }
        scheduler_sleep_ms(100);
    }
}

process_t *process_find_child(process_t *parent, pid_t child_pid)
{
    process_t *child = parent->child;
    if (!child)
    {
        return NULL;
    }
    if (child->pid == child_pid)
    {
        return child;
    }
    while (child)
    {
        process_t *next = (process_t *)child->ladder.next;
        if (next->child != NULL)
        {
            process_t *result = process_find_child(next, child_pid);
            if (result != NULL)
            {
                return result;
            }
        }
        child = (process_t *)next->ladder.next;
    }
    return NULL;
}

pid_t process_waitpid(pid_t child_pid, int *wstatus, int options)
{
    process_t *parent = process_get_current();

    process_t *child = process_find_child(parent, child_pid);
    if (child == NULL)
    {
        return RETURN_FAILED;
    }

    if (child->state != PROC_STATE_ZOMBIE)
    {
        thread *parent_thread = scheduler_get_current();
        scheduler_block(parent_thread);
        parent->wait_for = child_pid;
        scheduler_yield();
    }

    if (wstatus)
    {
        *wstatus = child->exit_code.raw;

        // [31-24] RES
        // [23-16] exitcode
        // [15-8] exitcode
        // [7-0] signal
    }

    process_reap(child);
    return child_pid;
}
SYSCALL_DEFINE3(process_waitpid, pid_t, int *, int);

int process_parse_args(process_t *proc, char *argv[], char *envp[])
{
    int argc = 0;
    int envc = 0;
    size_t str_size = 0;

    for (size_t i = 0; i < MAX_ARGS; i++)
    {
        if (argv[i] == NULL)
        {
            break;
        }
        str_size += strlen(argv[i]) + 1;
        argc++;
    }

    if (envp != NULL)
    {
        for (size_t i = 0; i < MAX_ARGS; i++)
        {
            if (envp[i] == NULL)
            {
                break;
            }
            str_size += strlen(envp[i]) + 1;
            envc++;
        }
    }
    reg_t user_stack = (reg_t)proc->vma->start_stack;
    user_stack &= ~0xFULL;
    user_stack -= str_size;

    reg_t stack_base = user_stack;

    uint64_t table_size = sizeof(uint64_t) * 3;
    table_size += argc * sizeof(uint64_t);
    table_size += envc * sizeof(uint64_t);

    user_stack -= table_size;
    user_stack &= ~0xFULL;

    reg_t table_cursor = user_stack;
    *((uint64_t *)table_cursor) = (uint64_t)argc;
    table_cursor += sizeof(uint64_t);

    for (size_t i = 0; i < argc; i++)
    {
        size_t length = strlen(argv[i]) + 1;
        memcpy((void *)stack_base, argv[i], length);
        *((uint32_64 *)table_cursor) = (uint32_64)stack_base;
        log_debug(MODULE, "argv[%d] = %s @ %p", i, argv[i], stack_base);
        stack_base += length;
        table_cursor += sizeof(uint64_t);
    }

    *((uint64_t *)table_cursor) = 0;
    table_cursor += sizeof(uint64_t);

    if (envp != NULL)
    {
        for (int i = 0; i < envc; i++)
        {
            size_t length = strlen(envp[i]) + 1;
            memcpy((void *)stack_base, envp[i], length);
            *((uint64_t *)table_cursor) = (uint64_t)stack_base;
            stack_base += length;
            table_cursor += sizeof(uint64_t);
        }
    }

    *((uint64_t *)table_cursor) = 0;

    proc->vma->start_stack = (virt_addr)user_stack;

    return argc;
}

int process_wait(process_t *proc)
{
    while (proc->state != PROC_STATE_ZOMBIE)
    {
        scheduler_yield(); // yield until child is done
    }
    process_exit_code_t code = proc->exit_code;
    process_reap(proc);
    return code.raw;
}

int process_exec(char *proc_path, char *argv[], char *envp[], process_t *proc)
{
    log_info(MODULE, "process_exec(%p, %p, %p, %p)", proc_path, argv, envp, proc);
    paging_page_t current_page;
    paging_get_current_cr3(&current_page);

    char *path = proc_path;
    if (*proc_path != '/')
    {
        path = malloc(MAX_PATH_SIZE);
        strcpy(path, proc->volume);
        char *colon_char = strchr(path, ':');
        int colon_index = (int)(colon_char - path)
        str
    }

    log_info(MODULE, "loading program %s", path);
    process_load(path, proc);

    int argc = process_parse_args(proc, argv, envp); // shit with args

    thread *user = thread_create_user(proc, (uint64_t)proc->vma->start_stack); // thread
    proc->threads[proc->thread_count] = user;
    proc->thread_count++;

    paging_load_cr3(current_page); // load cr3

    process_run(proc); // add the thread to the scheduler

    return process_wait(proc);
}

void kernel_init_process(const char *path, char *argv[], char *envp[])
{
    paging_page_t current_page;
    paging_get_current_cr3(&current_page);

    process_t *proc = process_create(); // load program into memory
    process_load((char *)path, proc);

    current_process = proc;
    int argc = process_parse_args(proc, argv, envp); // shit with args

    thread *user = thread_create_user(proc, (uint64_t)proc->vma->start_stack); // thread
    proc->threads[proc->thread_count] = user;
    proc->thread_count++;

    paging_load_cr3(current_page); // load cr3

    process_run(proc); // add the thread to the scheduler

    waitpid_loop();
}

int process_execve(const char *path, char *argv[], char *envp[], syscall_info *info)
{
    fprintf(VFS_FD_DEBUG, "path = %s argv = %p envp = %p\n", path, argv[0], envp[0]);
    process_t *proc = process_get_current();

    return process_exec((char *)path, argv, envp, proc);

    return RETURN_ERROR;
}
SYSCALL_DEFINE3_REG(process_execve, const char *, char **, char **);

int process_kill(pid_t pid, int sig)
{
    log_debug(MODULE, "killing procress %u using %u", pid, sig);
    process_t *proc = pid_table[pid];
    if (sig == SIGKILL)
    {
        do_exit(137, proc);
        return RETURN_GOOD;
    }
    signal_enqueue(proc, sig);
    return RETURN_GOOD;
}

process_t *process_get_current()
{
    thread *t = scheduler_get_current();
    return t->proc;
}
pid_t process_get_pid()
{
    process_t *curr = process_get_current();
    return curr->pid;
}
SYSCALL_DEFINE0(process_get_pid);

void process_init()
{
    paging_unmap_page(kernel_page, KERNEL_VDSO_VIRT);

    log_debug(MODULE, "init process");
    next_pid = 0;

    signal_init();
}
