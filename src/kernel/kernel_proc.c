/*
 * File: kernel_proc.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process/process.h"
#include "thread/thread.h"

#include "drivers/tty/tty.h"
#include "dev/device.h"

#include "asm/cpu_arch.h"
#include "asm/mmu_arch.h"

#include "sched/sched.h"

#include "memory.h"

#include "kerrno.h"

#define MODULE "pid0"

status_t kernel_pid0(process_t *process)
{
    ENTER_FUNC("%p", process);
    process_t *process_pid1;
    proc_allocate(process, &process_pid1);
    proc_spawn(process, process_pid1);
    trace_debug(MODULE, "process_pid1 = %p, pid1 = %u", process_pid1, process_pid1->pid);

    char *argv[2] = {"/@USER/bin/INIT.ELF", NULL};
    thread_t *t;
    proc_kernel_execve(process_pid1, "/@USER/bin/INIT.ELF", argv, NULL, &t);

    thread_t *curr = thread_get_current();
    curr->timeslice = curr->timeslice_reset;

    inline_asm("cli");
    // paging_print_tree(process_pid1->vma->page_directory);
    // paging_print_tree(&kernel_page);
    inline_asm("sti");

    proc_add_thread(process_pid1, t);

    // proc_run(process_pid1);

    while (true)
    {
        ;
    }

    return KERRNO_SUCCESSES;
}

THREAD_WARPER(kernel_pid0, process_t *);

void kernel_start_pid0()
{
    ENTER_FUNC("", 0);
    process_t *process;
    proc_allocate(NULL, &process);
    // proc_spawn(NULL, process);
    process->vma = kzmalloc(sizeof(vma_memory_t));
    process->vma->page_directory = &kernel_page;

    vfs_open_file(&process->fd_table, "/@DEVFS/tty", NULL);
    vfs_open_file(&process->fd_table, "/@DEVFS/vtty", NULL);
    vfs_open_file(&process->fd_table, "/@DEVFS/vtty8", NULL);

    thread_t *main_thread = thread_create_kernel(THREAD_CALL_FUNC(kernel_pid0), (uintptr_t)process, 0);
    main_thread->is_main_thread = 1;
    main_thread->cpu_affinity = cpu_arch_get_current();
    main_thread->proc = process;
    main_thread->cpu_affinity->current_proc = process;
    main_thread->timeslice_reset = cpu_arch_get_current()->current->timeslice_reset;
    main_thread->timeslice = cpu_arch_get_current()->current->timeslice;
    proc_add_thread(process, main_thread);
    sched_yield();
}
