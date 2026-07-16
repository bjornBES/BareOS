/*
 * File: process.c
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process.h"
#include "process_args.h"
#include "loader.h"
#include "stdio.h"

#include "mm/pmm.h"
#include "mm/vmm/vmm.h"
#include "mm/memdefs.h"
#include "mm/mmu/mmu.h"

#include "threading/thread.h"
#include "threading/scheduling/scheduler.h"

#include "kernel/cpu.h"
#include "kernel/string.h"
#include "kernel/memory.h"
#include "kernel/mmu.h"
#include "kernel/ivt.h"
#include "kernel.h"

#include "VFS/vfs.h"
#include "VFS/path.h"
#include "debug/debug.h"

#include "errno/errno.h"
#include "signals/exit.h"
#include "signal/signal.h"

#include <defs.h>

#define MODULE "PROC"

process_t *pid_table[MAX_PROCESSES];

extern char stack_top;

pid_t next_pid = 0;

extern void hexdump(void *ptr, int len);

int page_fault_try_count = 0;

int process_user_page_fault(intr_frame_t *regs, mmu_fault_info *info)
{
    log_info(MODULE, "here");
    vaddr_t faulting_va = PAGE_ALIGN_DOWN(info->fault_addr);
    if (faulting_va == 0)
    {
        KernelPanic(MODULE, "NULL exception TODO:");
    }

    log_info(MODULE, "getting current_thread");
    thread_t *current_thread = scheduler_get_current();
    if (current_thread)
    {
        log_info(MODULE, "comes from thread %u", current_thread->tid);
    }

    process_t *current_process = current_thread->proc;
    if (current_process == NULL)
    {
        log_info(MODULE, "comes from process %u", next_pid - 1);
        current_process = pid_table[next_pid - 1];
        vma_t *vma = vma_find(current_process->vma, info->fault_addr);
        if (vma == NULL)
        {
            log_info(MODULE, "%p has no vma entry", info->fault_addr);
            process_kill(current_process->pid, SIGSEGV);
            return RETURN_GOOD;
        }
        if (!info->present)
        {
            log_info(MODULE, "here1");
            paddr_t frame = pmm_alloc_frame();
            if (FLAGS_TO_RAW(vma->flags) == 0)
            {
                log_info(MODULE, "%p has no vma entry", info->fault_addr);
                process_kill(current_process->pid, SIGSEGV);
            }
            int state = mmu_arch_map(current_process->page_dir, faulting_va, frame, vma->flags);
            memset((void *)faulting_va, 0, PAGE_SIZE); // demand-zero
            return state;                              // resume faulting instruction
        }
        log_warn(MODULE, "current process is null");
        return RETURN_FAILED;
    }

    log_info(MODULE, "current cpuid is %u", cpu_arch_get_current()->cpu_id);
    log_info(MODULE, "current apic_id is %u", cpu_arch_get_current()->apic_id);

    if (current_process != NULL)
    {
        vma_t *vma = vma_find(current_process->vma, info->fault_addr);
        paddr_t addr = mmu_arch_virt_to_phys(current_process->page_dir, faulting_va);
        if (vma == NULL)
        {
            log_info(MODULE, "%p has no vma entry", info->fault_addr);
            process_kill(current_process->pid, SIGSEGV);
            return RETURN_GOOD;
        }
        else
        {
            if (!info->present)
            {
                log_info(MODULE, "here1");
                paddr_t frame = pmm_alloc_frame();
                if (FLAGS_TO_RAW(vma->flags) == 0)
                {
                    log_info(MODULE, "%p has no vma entry", info->fault_addr);
                    process_kill(current_process->pid, SIGSEGV);
                }
                // vma->flags.user = info->user;
                vma->flags.user = 1;
                int state = mmu_arch_map(current_process->page_dir, faulting_va, frame, vma->flags);
                memset((void *)faulting_va, 0, PAGE_SIZE); // demand-zero
                log_info(MODULE, "done with state %i", state);
                return state;                              // resume faulting instruction
            }
            if (info->is_cow && info->write)
            {
                log_debug(MODULE, "COW it to page dir %p", current_process->page_dir);
                paddr_t new_phys = pmm_alloc_frame();
                if (new_phys == 0)
                {
                    new_phys = pmm_alloc_frame();
                    if (new_phys == 0)
                    {
                        pmm_print_info_verbose();
                        log_err(MODULE, "OOM");
                        KernelPanic(MODULE, "OOM");
                    }
                }

                mmu_copy_contents(addr, new_phys);
                pmm_deref_frame(addr); // decrement old frame's refcount
                mmu_arch_unmap(current_process->page_dir, PAGE_ALIGN_DOWN(faulting_va));

                // remap this VA to new_phys with write=1, cow=0
                mmu_flags_t new_flags = info->entry_flags;
                new_flags.write = 1;
                new_flags.cow = 0;
                new_flags.user = 1;
                int state = mmu_arch_map(current_process->page_dir, PAGE_ALIGN_DOWN(faulting_va), new_phys, new_flags);
                mmu_arch_flush_page(faulting_va);
                log_debug(MODULE, "DONE with %i", state);
                // hexdump((uint64_t *)faulting_va, PAGE_SIZE);
                return state;
            }
        }

        if (addr != 0 && info->present && page_fault_try_count < 3)
        {
            page_fault_try_count++;
            log_info(MODULE, "%p Page is present", info->fault_addr);
            return RETURN_GOOD;
        }
        page_fault_try_count = 0;

        fprintf(VFS_FD_DEBUG, "[Page Fault] User process from virt address %p and phys address %p\n", info->fault_addr, addr);
        fprintf(VFS_FD_DEBUG, "[Page Fault] entry flags 0x%x\n", info->entry_flags);
        fprintf(VFS_FD_DEBUG, "[Page Fault] write %s\n", info->write BOOL_TO_STRING);
        fprintf(VFS_FD_DEBUG, "[Page Fault] user %s\n", info->user BOOL_TO_STRING);
        fprintf(VFS_FD_DEBUG, "[Page Fault] present %s\n", info->present BOOL_TO_STRING);
        fprintf(VFS_FD_DEBUG, "[Page Fault] exec %s\n", info->exec BOOL_TO_STRING);
        fprintf(VFS_FD_DEBUG, "[Page Fault] is cow page %s\n", info->is_cow BOOL_TO_STRING);

        thread_t *current_thread = scheduler_get_current();
        log_info(MODULE, "comes from thread %u", current_thread->tid);

        if (info->pc >= (uint64_t)KERNEL_VIRT_BASE)
        {
            schedule(NULL);
        }

        if (info->as_kernel)
        {
            fprintf(VFS_FD_DEBUG, "[Page Fault] active is kernels (%p)\n", kernel_page);
        }
        log_info(MODULE, "came from %p", info->page_directory.page_dir_phys);

        if (current_thread->state == THREAD_DEAD)
        {
            KernelPanic("Page Fault", "Got page fault from %p", info->fault_addr);
        }

        if (current_thread->tid != 0 && current_thread->state != THREAD_DEAD)
        {
            scheduler_thread_exit();
            return RETURN_GOOD;
        }

        pmm_print_info_verbose();

        fprintf(VFS_FD_DEBUG, "[Page Fault] Virtual address %p\n", info->fault_addr);
        fprintf(VFS_FD_DEBUG, "[Page Fault] Physical address %p\n", mmu_arch_virt_to_phys(&kernel_page, (vaddr_t)info->fault_addr));

        if (info->as_kernel)
        {
            KernelPanic("Page Fault", "Got page fault from %p", info->fault_addr);
        }
    }
    KernelPanic("Page Fault", "Got page fault from %p", info->fault_addr);
    // process_kill(current_process->pid, SIGSEGV);
    return RETURN_GOOD;
}

process_t *process_create()
{
    process_t *proc = malloc(sizeof(process_t));
    memset(proc, 0, sizeof(process_t));

    log_info(MODULE, "creating new user directory");
    proc->page_dir = mmu_arch_create_table();
    // mmu_arch_map_kernel(proc->page_dir);
    mmu_arch_load_table(proc->page_dir);
    log_info(MODULE, "user directory at v%p/p%p", proc->page_dir->page_dir, proc->page_dir->page_dir_phys);

    vma_init(proc);

    /* log_info(MODULE, "mmap stack proc->vma = %p", proc->vma);
    vma_t *stack = do_mmap_eager(proc->vma, USER_STACK_TOP, PAGE_SIZE * 2, VMA_ANONYMOUS, stack_flags);
    log_info(MODULE, "stack @ %p = {%p, %p}", stack, stack->start, stack->end);
    log_info(MODULE, "mmap heap proc->vma = %p", proc->vma);
    vma_t *heap = do_mmap_eager(proc->vma, USER_HEAP_BASE, PAGE_SIZE * 2, VMA_ANONYMOUS, data_flags);

    proc->vma->start_stack = stack->end;
    proc->vma->start_heap = heap->start;
    proc->vma->end_heap = heap->end;
    proc->vma->heap_vma = heap;
    proc->vma->stack_vma = stack; */

    log_info(MODULE, "mapping signals mapping %p to %p", KERNEL_VDSO_PHYS, USER_VDSO_VIRT);
    // kernel side, when setting up a new process
    mmu_arch_map(proc->page_dir, USER_VDSO_VIRT, KERNEL_VDSO_PHYS, vdso_text_flags);

    proc->pid = next_pid;
    pid_table[next_pid] = proc;
    next_pid = next_pid + 1;
    return proc;
}

void process_destroy(process_t *process)
{
    vma_free(process);
    mmu_arch_destroy_table(process->page_dir);
    free(process);
}

int process_load(char *path, process_t *proc)
{
    char volume[MAX_VOLUME_NAME];
    if (path_split_prefix(path, volume, NULL) == RETURN_GOOD)
    {
        strcpy(proc->volume, volume);
    }

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

process_t *process_clone(process_t *parent)
{
    process_t *child = kmalloc(sizeof(process_t));
    if (!child)
    {
        return NULL;
    }
    memset(child, 0, sizeof(process_t));

    child->pid = next_pid;
    pid_table[next_pid] = child;
    next_pid = next_pid + 1;
    child->parent = parent;

    strncpy(child->volume, parent->volume, sizeof(child->volume));

    child->thread_count = 0;
    memset(child->threads, 0, sizeof(thread_t *) * MAX_THREADS_PER_PROCESS);

    child->exit_code.raw = 0;
    child->state = PROC_STATE_RUNNING;

    child->pledge_mask = parent->pledge_mask;

    log_info(MODULE, "creating new user directory");
    child->page_dir = mmu_arch_create_table();
    child->vma = vma_clone(parent, child);
    child->vma->page_directory = child->page_dir;
    mmu_arch_map_kernel(child->page_dir);
    log_info(MODULE, "done cloning");

    log_info(MODULE, "signals");
    memcpy(&child->signal_table, &parent->signal_table, sizeof(signal_table));
    memset(&child->signal_queue, 0, sizeof(signal_pending));
    mmu_arch_map(child->page_dir, USER_VDSO_VIRT, KERNEL_VDSO_PHYS, vdso_text_flags);
    log_info(MODULE, "DONE signals");

    return child;
}

pid_t process_fork(syscall_info *info)
{
    // 1.
    thread_t *parent_thread = scheduler_get_current();
    process_t *parent = parent_thread->proc;

    process_t *child = process_clone(parent);
    if (!child)
    {
        return -ENOMEM;
    }

    memset(&child->ladder, 0, sizeof(task_ladder_t));
    child->parent = parent;
    child->child = NULL;

    thread_t *child_thread = thread_create_from(parent_thread, info->regs, child->vma->start_stack);
    log_info(MODULE, "walk process tree");
    if (!child_thread)
    {
        process_destroy(child);
        return -ENOMEM;
    }
    child_thread->proc = child;

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

    child->threads[0] = child_thread;
    child->thread_count++;

    scheduler_add(child_thread);

    mmu_arch_map_kernel(child->page_dir);

    log_debug(MODULE, "forked %u from %u", child->pid, parent->pid);
    return child->pid;
}

SYSCALL_DEFINE0_REG(process_fork);

void process_run(process_t *proc)
{
    log_debug(MODULE, "USER: stack = %p", proc->vma->start_stack);
    log_debug(MODULE, "jumping to %p", proc->entry);

    mmu_arch_map_kernel(proc->page_dir);

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
    mmu_arch_unmap(proc->page_dir, USER_VDSO_VIRT);
    mmu_arch_destroy_table(proc->page_dir);
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
        process_t *proc = process_get_current();
        if (proc != NULL)
        {
            process_t *child = proc->child;
            while (child)
            {
                process_t *next = (process_t *)child->ladder.next;
                if (child->state == PROC_STATE_ZOMBIE)
                {
                    process_reap(child);
                }
                child = next;
            }
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
        thread_t *parent_thread = scheduler_get_current();
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
    ENTER_FUNC(MODULE, "%p, %p, %p, %p", proc_path, argv, envp, proc);
    page_table_t current_page;
    mmu_arch_current_table(&current_page);

    process_args_t *info = NULL;
    bool kernel_start = false;
    if (proc == NULL)
    {
        kernel_start = true;
        proc = process_create(); // load program into memory
        log_info(MODULE, "saving args");
        info = process_args_save(proc, argv, envp);
    }
    else
    {
        log_info(MODULE, "saving args");
        info = process_args_save(proc, argv, envp);

        vma_free(proc);

        vma_init(proc);
    }

    char *path = proc_path;
    log_debug(MODULE, "path = %s", path);
    if (path_has_volume(proc_path) != RETURN_GOOD)
    {
        log_debug(MODULE, "proc->volume = %s", proc->volume);
        path = malloc(MAX_PATH_SIZE);
        int count = sprintf(path, "/%s:%s", proc->volume, proc_path);
        path[count] = '\0';
        log_debug(MODULE, "path = %s", path);
    }
    strcpy(proc->path, path);

    log_info(MODULE, "loading program %s", path);
    process_load(path, proc);

    log_info(MODULE, "loading args");
    process_args_parse(info, proc); // shit with args

    log_info(MODULE, "new thread");
    thread_t *user = thread_create_user(proc, (uint64_t)proc->vma->start_stack); // thread_t
    proc->threads[proc->thread_count] = user;
    proc->thread_count++;

    mmu_arch_flush_all();
    mmu_arch_load_table(&current_page); // load cr3

    process_run(proc);                  // add the thread_t to the scheduler

    if (kernel_start == true)
    {
        waitpid_loop();
        return RETURN_GOOD;
    }

    return process_wait(proc);
}

// void kernel_init_process(const char *path, char *argv[], char *envp[])
// {
//     page_table_t current_page;
//     mmu_arch_current_table(&current_page);

//     process_t *proc = process_create(); // load program into memory
//     process_load((char *)path, proc);

//     process_parse_args(proc, argv, envp);                                        // shit with args

//     thread_t *user = thread_create_user(proc, (uint64_t)proc->vma->start_stack); // thread_t
//     proc->threads[proc->thread_count] = user;
//     proc->thread_count++;

//     mmu_arch_flush_all();
//     mmu_arch_load_table(&current_page); // load table
//     mmu_arch_flush_all();

//     process_run(proc); // add the thread_t to the scheduler

//     log_debug(MODULE, "here4");

//     waitpid_loop();
// }

int process_execve(const char *path, char *argv[], char *envp[], syscall_info *info)
{
    fprintf(VFS_FD_DEBUG, "path = %s argv = %p envp = %p\n", path, argv[0], envp[0]);
    process_t *proc = process_get_current();

    return process_exec((char *)path, argv, envp, proc);
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
    signal_send_group(proc, sig);
    return RETURN_GOOD;
}

process_t *process_get_current()
{
    thread_t *t = scheduler_get_current();
    // log_debug(MODULE, "t @ %p, t->proc @ %p", t, t->proc);
    return t->proc;
}

pid_t process_get_pid()
{
    thread_t *t = scheduler_get_current();
    // log_debug(MODULE, "get pid from %u", t->proc->pid);
    return t->proc->pid;
}
SYSCALL_DEFINE0(process_get_pid);

pid_t proc_get_ppid()
{
    thread_t *t = scheduler_get_current();
    // log_debug(MODULE, "get pid from %u", t->proc->pid);
    return t->proc->parent->pid;
}
SYSCALL_DEFINE0(proc_get_ppid);


void process_init()
{
    mmu_arch_unmap(&kernel_page, KERNEL_VDSO_VIRT);

    log_debug(MODULE, "init process");
    next_pid = 0;

    signal_init();
}
