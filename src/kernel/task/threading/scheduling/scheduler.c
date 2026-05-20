/*
 * File: scheduler.c
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "scheduler.h"
#include "task/threading/spinlock/spinlock.h"
#include "task/threading/thread_config.h"
#include "task/threading/thread_type.h"
#include "task/threading/thread.h"

#include "kernel/threading/context.h"
#include "kernel/threading/threading.h"
#include "kernel/exceptions/exception.h"
#include "kernel/smp/cpu.h"
#include "kernel/task/tss.h"

#include "time/timer.h"
#include "kernel/irq.h"

#include "memory/paging/paging.h"

#include "libs/malloc.h"

#include "task/process.h"
#include "debug/debug.h"

#include <stddef.h>

#define MODULE "SCHEDULER"

#define current_thread (cpu_get_current()->current)

static thread_t *queue[MAX_THREADS] = {0};
static uint32_t queue_size = 0;

static thread_t *sleep_queue[MAX_THREADS] = {0};
static uint32_t sleep_queue_size = 0;

static thread_t *blocked_queue[MAX_THREADS] = {0};
static uint32_t blocked_queue_size = 0;

static uint32_t queue_head = 0;

void scheduler_init(thread_t *main_thread)
{
    log_info(MODULE, "set main thread");
    main_thread->state = THREAD_RUNNING;
    cpu_t *cpu = cpu_get_current();
    cpu->current = main_thread;
    queue[0] = main_thread;
    queue_size = 1;
    queue_head = 0;
}

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
        registers *next_regs = (registers *)candidate->ctx.stack_pointer;
        fprintf(VFS_FD_DEBUG, "thread[%u] = %p { tid: %u, state: %u, stack_base: %p, proc: %p, %p}\n", i, candidate, candidate->tid, candidate->state, candidate->stack_base, candidate->proc, next_regs);
        if (next_regs != NULL)
        {
            fprintf(VFS_FD_DEBUG, "\t{ ax = 0x%llx, bx = 0x%llx, cx = 0x%llx, dx = %llx }\n", next_regs->ax, next_regs->bx, next_regs->cx, next_regs->dx);
            fprintf(VFS_FD_DEBUG, "\t{ di = 0x%llx, si = 0x%llx, r8 = 0x%llx, r9 = %llx }\n", next_regs->di, next_regs->si, next_regs->r8, next_regs->r9);
            fprintf(VFS_FD_DEBUG, "\t{ r10 = 0x%llx, r11 = 0x%llx, r12 = 0x%llx, r13 = %llx }\n", next_regs->r10, next_regs->r11, next_regs->r12, next_regs->r13);
            fprintf(VFS_FD_DEBUG, "\t{ r14 = 0x%llx, r15 = 0x%llx, bp = 0x%llx, flags = %llx }\n", next_regs->r14, next_regs->r15, next_regs->bp, next_regs->flags);
            fprintf(VFS_FD_DEBUG, "\t{ pc = 0x%x:%p, sp = 0x%x:%p }\n", next_regs->cs, next_regs->pc, next_regs->ss, next_regs->sp);
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
            registers *next_regs = (registers *)candidate->ctx.stack_pointer;
            fprintf(VFS_FD_DEBUG, "thread[%u] = %p { tid: %u, state: %u, stack_base: %p, proc: %p, %p}\n", i, candidate, candidate->tid, candidate->state, candidate->stack_base, candidate->proc, next_regs);
            if (next_regs != NULL)
            {
                fprintf(VFS_FD_DEBUG, "\t{ ax = 0x%llx, bx = 0x%llx, cx = 0x%llx, dx = %llx }\n", next_regs->ax, next_regs->bx, next_regs->cx, next_regs->dx);
                fprintf(VFS_FD_DEBUG, "\t{ di = 0x%llx, si = 0x%llx, r8 = 0x%llx, r9 = %llx }\n", next_regs->di, next_regs->si, next_regs->r8, next_regs->r9);
                fprintf(VFS_FD_DEBUG, "\t{ r10 = 0x%llx, r11 = 0x%llx, r12 = 0x%llx, r13 = %llx }\n", next_regs->r10, next_regs->r11, next_regs->r12, next_regs->r13);
                fprintf(VFS_FD_DEBUG, "\t{ r14 = 0x%llx, r15 = 0x%llx, bp = 0x%llx, flags = %llx }\n", next_regs->r14, next_regs->r15, next_regs->bp, next_regs->flags);
                fprintf(VFS_FD_DEBUG, "\t{ pc = 0x%x:%p, sp = 0x%x:%p }\n", next_regs->cs, next_regs->pc, next_regs->ss, next_regs->sp);
            }
        }
        fprintf(VFS_FD_DEBUG, "===== Sleep queue =====\n");
    }
}

void scheduler_add(thread_t *t)
{
    log_debug(MODULE, "adding thread %u", t->tid);
    log_debug(MODULE, "thread = %p { tid: %u, state: %u, stack_base: %p, proc: %p}", t, t->tid, t->state, t->stack_base, t->proc);
    if (queue_size > MAX_THREADS)
    {
        bool has_space = false;
        for (uint32_t i = 0; i < MAX_THREADS; i++)
        {
            thread_t *candidate = queue[i];
            if (candidate == NULL)
            {
                has_space = true;
                queue_size++;
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
            log_debug(MODULE, "thread[%u] = %p { tid: %u, state: %u, stack_base: %p, proc: %p}", i, candidate, candidate->tid, candidate->state, candidate->stack_base, candidate->proc);
        }
        if (queue[i] == NULL)
        {
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
            log_info(MODULE, "found %u", t->tid);
            THREAD_EXIT(queue[i]);
            queue[i] = NULL;
            return;
        }
    }
}

void scheduler_sleep_ms(time_t ms)
{
    current_thread->wake_time = timer_elapsed_time(0) + ms;
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

    __asm__("int 0x22");
}
void scheduler_wakeup_check()
{
    time_t now = timer_elapsed_time(0);
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
            log_debug(MODULE, "thread[%u] = %p { tid: %u, state: %u, stack_base: %p, proc: %p}", idx, candidate, candidate->tid, candidate->state, candidate->stack_base, candidate->proc);
            queue[idx] = NULL;
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
    cpu_t *cpu = cpu_get_current();
    if (cpu == NULL)
    {
        uint32_t lo, hi;
        __asm__ volatile(
            "mov ecx, 0xC0000101\n"
            "rdmsr\n"
            : "=a"(lo), "=d"(hi)
            :
            : "ecx");
        log_crit(MODULE, "CPU is null getting %08x%08x", hi, lo);
    }
    return cpu->current;
}

int scheduler_yield()
{
    __asm__("int 0x22");
    return RETURN_GOOD;
}
SYSCALL_DEFINE0(scheduler_yield);

spinlock_t schedule_lock;
static uint64_t bsp_ticks = 0;
void schedule(registers *regs)
{
    // spinlock_acquire(&schedule_lock);
    cpu_t *cpu = cpu_get_current();
    if (cpu == NULL)
    {
        driver->send_eoi(0);
        return;
    }
    if (cpu->current == NULL)
    {
        driver->send_eoi(0);
        return;
    }

    if (cpu->cpu_id == 0)
    {
        bsp_ticks++;
        if (bsp_ticks % 100 == 0)
        {
            // fprintf(VFS_FD_DEBUG, "[SCHEDULER] cpu0\n");
            // fprintf(VFS_FD_DEBUG, "[SCHEDULER] BSP tick=%u current=t%u\n", bsp_ticks, current_thread->tid);
        }
    }

    scheduler_wakeup_check();

    if (current_thread->state == THREAD_RUNNING)
    {
        if (current_thread->timeslice > 0)
        {
            current_thread->timeslice--;
        }

        if (current_thread->timeslice > 0)
        {
            driver->send_eoi(0);
            return;
        }
    }

    // log_debug(MODULE, "try to schedule");

    thread_t *next = scheduler_next();
    if (next == current_thread)
    {
        current_thread->timeslice = current_thread->timeslice_reset; // reload
        driver->send_eoi(0);
        return;
    }

    if (regs != NULL)
    {
        /*         log_debug(MODULE, "switch from t%u -> t%u regs = %p", current_thread->tid, next->tid, regs);
                log_debug(MODULE, "switch from t%u stack_base = %p", next->tid, next->ctx.stack_pointer);
                log_debug(MODULE, "switch from t%u stack_base = %p", current_thread->tid, current_thread->ctx.stack_pointer); */
        current_thread->ctx.stack_pointer = (uint64_t)regs;
    }
    else
    {
        /* log_debug(MODULE, "switching to t%u regs = %p", next->tid, regs); */
        scheduler_thread_info();
    }
    registers *next_regs = (registers *)next->ctx.stack_pointer;
    /*     fprintf(VFS_FD_DEBUG, "ax=0x%llx bx=0x%llx cx=0x%llx dx=0x%llx si=0x%llx di=0x%llx\nsp=0x%llx bp=0x%llx pc=0x%04x:0x%llx flags=0x%x cs=0x%x ds=0x%x ss=0x%x\n",
                next_regs->ax, next_regs->bx, next_regs->cx, next_regs->dx, next_regs->si, next_regs->di,
                next_regs->sp, next_regs->bp, next_regs->cs, next_regs->pc, next_regs->flags, next_regs->cs, next_regs->ds, next_regs->ss);
        log_debug(MODULE, "here1"); */
    if (next->proc != current_thread->proc)
    {
        /*         log_debug(MODULE, "here2");
                log_debug(MODULE, "curr.proc %p next.proc %p", current_thread->proc, next->proc); */
        if (next->tid == 0 || next->proc == NULL)
        {
            // fprintf(VFS_FD_STDOUT, "switch to kernel\n");
            paging_load_cr3(kernel_page);
        }
        else
        {
            // fprintf(VFS_FD_STDOUT, "switch to process %u\n", next->proc->pid);
            paging_load_cr3(next->proc->page_dir);
        }
        log_debug(MODULE, "here3");
    }

    tss_set_kernel_sp((reg_t)next->stack_base);

    thread_t *old = current_thread;
    current_thread = next;
    current_thread->state = THREAD_RUNNING;
    current_thread->timeslice = current_thread->timeslice_reset;
    /*     log_debug(MODULE, "t%u running for %u cycles", current_thread->tid, current_thread->timeslice); */

    if (old->state == THREAD_RUNNING)
    {
        old->state = THREAD_READY;
    }
    // spinlock_release(&schedule_lock);
    driver->send_eoi(0);
    /*     log_debug(MODULE, "send eoi"); */
    context_switch(current_thread->ctx.stack_pointer);
}

void scheduler_thread_exit()
{
    log_debug(MODULE, "marking t%u as dead", current_thread->tid);
    current_thread->state = THREAD_REMAINS;
    log_debug(MODULE, "find new");
    schedule(NULL); // never returns
}