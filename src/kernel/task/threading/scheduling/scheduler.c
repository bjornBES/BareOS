/*
 * File: scheduler.c
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "scheduler.h"
#include "task/threading/spinlock/spinlock.h"
#include "task/threading/thread_config.h"
#include "task/threading/thread_type.h"
#include "task/threading/thread.h"

#include "kernel/ctx.h"
#include "kernel/threading/threading.h"
#include "kernel/threading/scheduler.h"
#include "kernel/cpu.h"
#include "kernel/mmu.h"
#include "kernel/irq.h"
#include "kernel/ivt.h"
#include "kernel/memory.h"
#include "kernel.h"

#include "time/timer.h"

#include "task/process.h"
#include "debug/debug.h"

#include "math.h"
#include <stddef.h>

#define MODULE "SCHEDULER"

#define SCHEDULER_TICK_NS 1000000ull // 1ms

#define current_thread (cpu_arch_get_current()->current)

uint32_t total_threads = 0;

static thread_t *queue[MAX_THREADS] = {0};
static uint32_t queue_size = 0;

static thread_t *sleep_queue[MAX_THREADS] = {0};
static uint32_t sleep_queue_size = 0;

static thread_t *blocked_queue[MAX_THREADS] = {0};
static uint32_t blocked_queue_size = 0;

static uint32_t queue_head = 0;

void scheduler_thread_info()
{
    fprintf(VFS_FD_DEBUG, "\n===== Active queue =====\n");
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        thread_t *candidate = queue[i];
        if (candidate == NULL)
        {
            fprintf(VFS_FD_DEBUG, "thread[%u] = %p\n", i, candidate);
            continue;
        }
        fprintf(VFS_FD_DEBUG, "thread[%u] = %p { tid: %u, state: %u, kernel_stack: %p, proc: %p, %p}\n", i, candidate, candidate->tid, candidate->state, candidate->kernel_stack, candidate->proc, candidate->ctx.frame.sp);
        if (candidate->ctx.frame.regs != NULL)
        {
            ctx_dump(&candidate->ctx);
        }
    }
    fprintf(VFS_FD_DEBUG, "===== Active queue =====\n");

    if (sleep_queue_size != 0)
    {
        fprintf(VFS_FD_DEBUG, "\n===== Sleep queue =====\n");
        fprintf(VFS_FD_DEBUG, "Sleep queue\n");
        for (uint32_t i = 0; i < MAX_THREADS; i++)
        {
            thread_t *candidate = sleep_queue[i];
            if (candidate == NULL)
            {
                fprintf(VFS_FD_DEBUG, "thread[%u] = %p\n", i, candidate);
                continue;
            }
            fprintf(VFS_FD_DEBUG, "thread[%u] = %p { tid: %u, state: %u, kernel_stack: %p, proc: %p, %p}\n", i, candidate, candidate->tid, candidate->state, candidate->kernel_stack, candidate->proc, candidate->ctx.frame.sp);
            if (candidate->ctx.frame.regs != NULL)
            {
                ctx_dump(&candidate->ctx);
            }
        }
        fprintf(VFS_FD_DEBUG, "===== Sleep queue =====\n");
    }

    if (blocked_queue_size != 0)
    {
        fprintf(VFS_FD_DEBUG, "\n===== block queue =====\n");
        fprintf(VFS_FD_DEBUG, "Block queue\n");
        for (uint32_t i = 0; i < MAX_THREADS; i++)
        {
            thread_t *candidate = blocked_queue[i];
            if (candidate == NULL)
            {
                fprintf(VFS_FD_DEBUG, "thread[%u] = %p\n", i, candidate);
                continue;
            }
            fprintf(VFS_FD_DEBUG, "thread[%u] = %p { tid: %u, state: %u, kernel_stack: %p, proc: %p, %p}\n", i, candidate, candidate->tid, candidate->state, candidate->kernel_stack, candidate->proc, candidate->ctx.frame.sp);
            if (candidate->ctx.frame.regs != NULL)
            {
                ctx_dump(&candidate->ctx);
            }
        }
        fprintf(VFS_FD_DEBUG, "===== block queue =====\n");
    }
}

void scheduler_add(thread_t *t)
{
    log_debug(MODULE, "adding thread %u", t->tid);
    log_debug(MODULE, "thread = %p { tid: %u, state: %u, kernel_stack: %p, proc: %p}", t, t->tid, t->state, t->kernel_stack, t->proc);
    if (queue_size > MAX_THREADS)
    {
        bool has_space = false;
        for (uint32_t i = 0; i < MAX_THREADS; i++)
        {
            thread_t *candidate = queue[i];
            if (candidate == NULL)
            {
                has_space = true;
            }
        }
        if (has_space == false)
        {
            scheduler_thread_info();
            KernelPanic(MODULE, "out of threads");
            return; // or panic
        }
    }

    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        thread_t *candidate = queue[i];
        if (candidate != NULL)
        {
            log_debug(MODULE, "thread[%u] = %p { tid: %u, state: %u, kernel_stack: %p, proc: %p}", i, candidate, candidate->tid, candidate->state, candidate->kernel_stack, candidate->proc);
        }
        if (queue[i] == NULL)
        {
            total_threads++;
            log_debug(MODULE, "found thread[%u]", i);
            queue[i] = t;
            queue_size++;
            return;
        }
    }
}

void scheduler_remove(thread_t *t)
{
    log_debug(MODULE, "removing thread %u", t->tid);

    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        thread_t *candidate = queue[i];
        if (candidate != NULL)
        {
            log_debug(MODULE, "candidate[%u] @ %p = {state = %u, tid = %u}", i, candidate, candidate->state, candidate->tid);
        }
        if (queue[i] != NULL && queue[i]->tid == t->tid)
        {
            if (t->state == THREAD_RUNNING)
            {
                t->state = THREAD_REMAINS;
                continue;
            }
            total_threads--;
            log_info(MODULE, "found %u", t->tid);
            THREAD_EXIT(queue[i]);
            queue[i] = NULL;
            return;
        }
    }
}

void scheduler_sleep(uint64_t ns)
{
    current_thread->wake_time = timer_now_ns() + ns;
    current_thread->state = THREAD_BLOCKED;
    // add to sleep queue
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (sleep_queue[i] == NULL)
        {
            sleep_queue[i] = current_thread;
            current_thread->state = THREAD_SLEEP;
            sleep_queue_size++;
            break;
        }
    }
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (queue[i] == current_thread)
        {
            queue[i] = NULL;
        }
    }

    scheduler_yield();
}

void scheduler_sleep_ms(uint64_t ms)
{
    scheduler_sleep(MS_TO_NS(ms));
}

void scheduler_sleep_sec(time_t sec)
{
    scheduler_sleep(SEC_TO_NS(sec));
}

void scheduler_wakeup_check()
{
    uint64_t now = timer_now_ns();
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        thread_t *t = sleep_queue[i];
        if (t == NULL)
        {
            continue;
        }
        if (now >= t->wake_time)
        {
            log_debug(MODULE, "t%u WAKE UP", t->tid);
            t->state = THREAD_JUST_WOKE;
            t->wake_time = 0;
            sleep_queue[i] = NULL;
            sleep_queue_size--;
            scheduler_add(t);
        }
    }
}

void scheduler_block(thread_t *t)
{
    // remove from run queue
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (queue[i] == t)
        {
            queue[i] = NULL;
            queue_size--;
            break;
        }
    }
    // add to blocked queue
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (blocked_queue[i] == NULL)
        {
            blocked_queue[i] = t;
            blocked_queue_size++;
            break;
        }
    }
    t->state = THREAD_BLOCKED;

    scheduler_thread_info();
}

void scheduler_unblock(thread_t *t)
{
    // remove from blocked queue
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (blocked_queue[i] == t)
        {
            blocked_queue[i] = NULL;
            blocked_queue_size--;
            break;
        }
    }
    t->state = THREAD_READY;
    scheduler_add(t);
}

thread_t *scheduler_find_waiting(process_t *proc)
{
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        thread_t *blocked_thread = blocked_queue[i];
        for (size_t j = 0; j < MAX_THREADS_PER_PROCESS; j++)
        {
            thread_t *process_thread = proc->threads[i];
            if (process_thread == blocked_thread)
            {
                return process_thread;
            }
        }
    }
    return NULL;
}

static thread_t *scheduler_next()
{
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        uint32_t idx = (queue_head + i) % MAX_THREADS;
        thread_t *candidate = queue[idx];

        if (candidate == NULL)
        {
            continue;
        }
        // log_debug(MODULE, "candidate[%u] @ %p = {state = %u, tid = %u}", i, candidate, candidate->state, candidate->tid);

        /*         if (candidate == current_thread && candidate->state == THREAD_REMAINS)
                {
                    log_debug(MODULE, "candidate[%u] and current_thread are both about to die", idx);
                    continue;
                } */

        if (candidate->state == THREAD_JUST_WOKE)
        {
            candidate->state = THREAD_READY;
            continue;
        }

        if (candidate->state == THREAD_REMAINS && candidate != current_thread)
        {
            log_debug(MODULE, "cleaning up thread @ candidate[%u]", idx);
            log_debug(MODULE, "thread[%u] = %p { tid: %u, state: %u, kernel_stack: %p, proc: %p}", idx, candidate, candidate->tid, candidate->state, candidate->kernel_stack, candidate->proc);
            queue[idx] = NULL;
            total_threads++;
            THREAD_EXIT(candidate);
            queue_size--;
            log_debug(MODULE, "cleaned up and done with candidate[%u]", idx);
            scheduler_thread_info();
            continue;
        }

        if (candidate->state == THREAD_READY)
        {
            queue_head = idx;
            return candidate;
        }
    }
    return queue[0]; // no other thread_t ready, run the main thread_t
}

thread_t *scheduler_get_current()
{
    cpu_t *cpu = cpu_arch_get_current();
    // log_debug(MODULE, "cpu @ %p, cpu->current @ %p", cpu, cpu->current);
    return cpu->current;
}

int scheduler_yield()
{
    return scheduler_arch_yield();
}

SYSCALL_DEFINE0(scheduler_yield);

void scheduler_tick()
{
    scheduler_wakeup_check();

    // rearm — next tick or next wakeup whichever sooner
    uint64_t next = SCHEDULER_TICK_NS;
    timer_set_oneshot(next, scheduler_tick);

    if (current_thread->state == THREAD_RUNNING)
    {
        if (current_thread->timeslice > 0)
        {
            current_thread->timeslice--;
        }

        if (current_thread->timeslice > 0)
        {
            scheduler_yield();
            return;
        }
    }
}

__attribute__((noreturn)) void schedule_switch(thread_t *next)
{

    thread_t *old = current_thread;
    current_thread = next;
    current_thread->state = THREAD_RUNNING;
    current_thread->timeslice = current_thread->timeslice_reset;

    if (old->state == THREAD_RUNNING)
    {
        old->state = THREAD_READY;
    }
    log_debug(MODULE, "here4");
    // spinlock_release(&schedule_lock);
    vaddr_t stack_pointer = ctx_arch_get_sp(&current_thread->ctx);
    page_table_t table;
    mmu_arch_current_table(&table);
    log_debug(MODULE, "here5 %p phys = %p", stack_pointer, mmu_arch_virt_to_phys(&table, stack_pointer));

    if (current_thread->ctx.frame.regs != NULL)
    {
        ctx_dump(&current_thread->ctx);
    }
    
    log_debug(MODULE, "here6 switching to 0x%lx", current_thread->kernel_stack);
    ivt_dump_frame(current_thread->ctx.frame.regs);
    irq_arch_eoi(0);
    ctx_arch_switch(current_thread->kernel_stack);

    log_debug(MODULE, "something is wrong");
    KernelPanic(MODULE, "something is wrong");

    // loop
    for (;;);
}

int schedule(intr_frame_t *regs)
{
    // spinlock_acquire(&schedule_lock);
    cpu_t *cpu = cpu_arch_get_current();
    if (cpu == NULL)
    {
        irq_arch_eoi(0);
        return RETURN_FAILED;
    }
    if (cpu->current == NULL)
    {
        irq_arch_eoi(0);
        return RETURN_FAILED;
    }

    thread_t *next = scheduler_next();
    if (next == current_thread)
    {
        current_thread->timeslice = current_thread->timeslice_reset; // reload
        irq_arch_eoi(0);
        return RETURN_GOOD;
    }

    if (regs != NULL)
    {
        // ctx_arch_set_sp(&current_thread->ctx, (vaddr_t)regs);
    }
    else
    {
        scheduler_thread_info();
    }
    if (next->proc != current_thread->proc)
    {
        if (next->tid == 0 || next->proc == NULL)
        {
            // fprintf(VFS_FD_STDOUT, "switch to kernel\n");
            mmu_arch_load_table(&kernel_page);
        }
        else
        {
            // fprintf(VFS_FD_STDOUT, "switch to process %u\n", next->proc->pid);
            mmu_arch_load_table(next->proc->page_dir);
        }
        log_debug(MODULE, "here3");
    }

    cpu_arch_set_kernel_stack((vaddr_t)next->kernel_stack);

    schedule_switch(next);
    return RETURN_GOOD;
}

void scheduler_thread_exit()
{
    log_debug(MODULE, "marking t%u as dead", current_thread->tid);
    current_thread->state = THREAD_REMAINS;
    log_debug(MODULE, "find new");
    schedule(NULL); // never returns
}

void scheduler_init(thread_t *main_thread)
{
    memcpy(main_thread->name, "MAIN\0", 4);
    log_info(MODULE, "setting T%u (%s) as the main thread", main_thread->tid, main_thread->name);
    main_thread->state = THREAD_RUNNING;
    cpu_t *cpu = cpu_arch_get_current();
    cpu->current = main_thread;
    queue[0] = main_thread;
    queue_size = 1;
    queue_head = 0;
    total_threads++;
    timer_set_oneshot(SCHEDULER_TICK_NS, scheduler_tick);
    ivt_arch_set_handler(0x7F, schedule);
}
