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
#include "kernel/smp.h"
#include "kernel/mmu.h"
#include "kernel/irq.h"
#include "kernel/ivt.h"
#include "kernel/memory.h"
#include "kernel.h"

#include "time/timer.h"

#include "task/process.h"
#include "debug/debug.h"

#include "math.h"
#include <lists/list.h>
#include <types.h>

#define MODULE            "SCHEDULER"

#define SCHEDULER_TICK_NS 1000000ull // 1ms

#define current_thread    (cpu_arch_get_current()->current)

typedef struct
{
    spinlock_t lock;
    list_t threads; // intrusive list_node_t, same pattern as your other lists
    int count;      // cheap "is it worth checking" hint before taking the lock
} global_runq_t;

global_runq_t sched_runq; // one instance, kernel-wide
global_runq_t sleep_queue; // one instance, kernel-wide

uint32_t total_threads = 0;

static thread_t *queue[MAX_THREADS] = {0};
static uint32_t queue_size = 0;

static thread_t *blocked_queue[MAX_THREADS] = {0};
static uint32_t blocked_queue_size = 0;

// static uint32_t queue_head = 0;

void sched_print_thread_info(thread_t *t)
{
    char *THREAD_TYPE_STRING[8] = {
        [THREAD_READY] = "READY",
        [THREAD_RUNNING] = "RUNNING",
        [THREAD_SLEEP] = "SLEEP",
        [THREAD_DEAD] = "DEAD",
        [THREAD_BLOCKED] = "BLOCKED",
        [THREAD_JUST_WOKE] = "JUST WOKE",
        [THREAD_REMAINS] = "REMAINS",
    };
    fprintf(VFS_FD_DEBUG, "thread = %p { tid: %u, state: %s, kernel_stack: %p, ", t, t->tid, THREAD_TYPE_STRING[t->state], t->kernel_stack);
    if (t->proc != NULL)
    {
        fprintf(VFS_FD_DEBUG, "proc: %p {pid: %u}, ", t->proc, t->proc->pid);
    }
    if (t->state == THREAD_SLEEP)
    {
        fprintf(VFS_FD_DEBUG, "wake_time: %u, ", t->wake_time);
    }
    fprintf(VFS_FD_DEBUG, "timeslice: %u }\n", t->timeslice);
}

void sched_thread_info()
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

static void sched_destroy_thread(thread_t *t, int i)
{
    log_debug(MODULE, "cleaning up thread @ queue[%u]", i);
    log_debug(MODULE, "thread[%u] = %p { tid: %u, state: %u, kernel_stack: %p, proc: %p}", i, t, t->tid, t->state, t->kernel_stack, t->proc);

    total_threads--;
    log_info(MODULE, "found %u", t->tid);
    THREAD_EXIT(t);
    queue[i] = NULL;
    queue_size--;

    log_debug(MODULE, "cleaned up and done with queue[%u]", i);
    sched_thread_info();
}

void sched_enqueue(thread_t *t)
{
    log_debug(MODULE, "enqueuing thread %u", t->tid);
    cpu_t *target = NULL;
    t->state = THREAD_READY;

    if (t->cpu_affinity)
    {
        target = t->cpu_affinity;
    }
    else if (t->last_cpu)
    {
        target = t->last_cpu;
    }

    if (target)
    {
        log_debug(MODULE, "in cpu %u", target->apic_id);
        spinlock_acquire(&target->local_runq_lock);
        list_push_tail(&target->local_runq, &t->node);
        target->local_count++;
        spinlock_release(&target->local_runq_lock);
        
        if (target != cpu_arch_get_current())
        {
            // todo: IPI, only needed cross-core
            smp_arch_send_ipi(target->apic_id, IPI_RESCHEDULE_VECTOR);
        }
    }
    else
    {
        log_debug(MODULE, "in global");
        spinlock_acquire(&sched_runq.lock);
        list_push_tail(&sched_runq.threads, &t->node);
        sched_runq.count++;
        spinlock_release(&sched_runq.lock);
    }
}

void sched_remove_from_list(thread_t *t)
{
    cpu_t *cpu = cpu_arch_get_current();

    spinlock_acquire(&cpu->local_runq_lock);
    if (list_remove(&cpu->local_runq, &t->node) == RETURN_GOOD)
    {
        log_debug(MODULE, "removed...");
        // sched_print_thread_info(t);
        cpu->local_count--;
        spinlock_release(&cpu->local_runq_lock);
        return;
    }
    spinlock_release(&cpu->local_runq_lock);

    spinlock_acquire(&sched_runq.lock);
    if (list_remove(&sched_runq.threads, &t->node) == RETURN_GOOD)
    {
        log_debug(MODULE, "removed...");
        sched_runq.count--;
        spinlock_release(&sched_runq.lock);
        return;
    }
    spinlock_release(&sched_runq.lock);
}

void sched_add(thread_t *t)
{
    log_debug(MODULE, "adding thread %u", t->tid);
    sched_print_thread_info(t);
    sched_enqueue(t);
    /*     if (queue_size > MAX_THREADS)
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
                sched_thread_info();
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
        } */
}

void sched_remove(thread_t *t)
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
            sched_destroy_thread(t, i);
            return;
        }
    }
}

void sleep_queue_insert(list_t *list, list_node_t *node, uint64_t wake_time)
{
    list_node_t *curr = list->head;

    while (curr != NULL)
    {
        thread_t *curr_thread = container_of(curr, thread_t, node);
        if (curr_thread->wake_time > wake_time)
        {
            break; // found the first node that wakes later — insert before it
        }
        curr = curr->next;
    }

    if (curr == NULL)
    {
        // reached the end — this is the latest wake time, goes at the tail
        list_push_tail(list, node);
        return;
    }

    if (curr == list->head)
    {
        list_push_head(list, node);
        return;
    }

    // insert node between curr->prev and curr
    list_node_t *prev = curr->prev;
    node->prev = prev;
    node->next = curr;
    prev->next = node;
    curr->prev = node;
    list->count++;
}

void sched_sleep(uint64_t ns)
{
    ENTER_FUNC(MODULE, "%u", ns);
    uint64_t wake_time = timer_now_ns() + ns;
    current_thread->state = THREAD_SLEEP;
    current_thread->wake_time = wake_time;
    
    // sched_print_thread_info(current_thread);
    sleep_queue_insert(&sleep_queue.threads, &current_thread->node, wake_time);
    sched_remove_from_list(current_thread);
    sched_print_thread_info(current_thread);

    sched_yield();
}

void sched_sleep_ms(uint64_t ms)
{
    sched_sleep(MS_TO_NS(ms));
}

void sched_sleep_sec(time_t sec)
{
    sched_sleep(SEC_TO_NS(sec));
}

void scheduler_wakeup_check()
{
    uint64_t now = timer_now_ns();
    spinlock_acquire(&sleep_queue.lock);
    while (sleep_queue.threads.head != NULL)
    {
        thread_t *t = container_of(sleep_queue.threads.head, thread_t, node);
        if (t->wake_time > now)
        {
            break; // sorted — if head isn't due, nothing after it is either
        }

        log_debug(MODULE, "thread ... is done sleeping");
        sched_print_thread_info(t);

        list_pop_head(&sleep_queue.threads); // actually remove it now
        spinlock_release(&sleep_queue.lock);

        t->wake_time = 0;
        t->state = THREAD_READY;
        sched_enqueue(t);                   // goes to last_cpu/affinity or global, as normal

        spinlock_acquire(&sleep_queue.lock); // re-lock to continue checking the new head
    }
    spinlock_release(&sleep_queue.lock);
}

void sched_block(thread_t *t)
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

    sched_thread_info();
}

void sched_unblock(thread_t *t)
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
    sched_add(t);
}

thread_t *sched_find_waiting(process_t *proc)
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

static thread_t *sched_next()
{
    cpu_t *me = cpu_arch_get_current();

    // 1. own local queue
    spinlock_acquire(&me->local_runq_lock);
    if (me->local_count > 0)
    {
        // log_debug(MODULE, "thread from cpu %u", me->cpu_id);
        GET_THREAD(me->local_runq);
        me->local_count--;
        sched_print_thread_info(thread);
        spinlock_release(&me->local_runq_lock);
        return thread;
    }
    spinlock_release(&me->local_runq_lock);
    // log_debug(MODULE, "next");

    // 2. global queue
    if (sched_runq.count > 0)
    {
        // cheap check before locking
        spinlock_acquire(&sched_runq.lock);
        if (sched_runq.count > 0)
        {
            // re-check after acquiring lock
            GET_THREAD(sched_runq.threads);
            sched_runq.count--;
            sched_print_thread_info(thread);
            spinlock_release(&sched_runq.lock);
            return thread;
        }
        spinlock_release(&sched_runq.lock);
    }
    // log_debug(MODULE, "next");

    // TODO steal

    // log_debug(MODULE, "NULL");
    return NULL; // caller goes idle (hlt)

    /*     for (uint32_t i = 0; i < MAX_THREADS; i++)
        {
            uint32_t idx = (queue_head + i) % MAX_THREADS;
            thread_t *candidate = queue[idx];

            if (candidate == NULL)
            {
                continue;
            }
            log_debug(MODULE, "candidate[%u] @ %p = {state = %u, tid = %u}", idx, candidate, candidate->state, candidate->tid);


            if (candidate->state == THREAD_JUST_WOKE)
            {
                candidate->state = THREAD_READY;
                continue;
            }

            if (candidate->state == THREAD_REMAINS && candidate != current_thread)
            {
                sched_destroy_thread(candidate, idx);
                continue;
            }

            if (candidate->proc)
            {
                if (candidate->proc->state == PROC_STATE_SUSPENDED)
                {
                    continue;
                }
            }

            if (candidate->state == THREAD_READY)
            {
                queue_head = idx;
                return candidate;
            }
        }
        return queue[0]; // no other thread_t ready, run the main thread_t */
}

thread_t *sched_get_current()
{
    // ENTER_FUNC(MODULE, "", "");
    cpu_t *cpu = cpu_arch_get_current();
    // log_debug(MODULE, "cpu @ %p", cpu);
    // log_debug(MODULE, "cpu->current @ %p", cpu->current);
    return cpu->current;
}

int sched_yield()
{
    // log_info(MODULE, "scheduler_yield");
    // sched_thread_info();
    return scheduler_arch_yield();
}

SYSCALL_DEFINE0(sched_yield);

void scheduler_tick(device_t *dev)
{
    fprintf(VFS_FD_DEBUG, "scheduler_tick\n");
    scheduler_wakeup_check();

    // rearm — next tick or next wakeup whichever sooner
    // uint64_t next = SCHEDULER_TICK_NS;
    // timer_set_oneshot(next, scheduler_tick);

    if (current_thread->state == THREAD_RUNNING)
    {
        if (current_thread->timeslice > 0)
        {
            current_thread->timeslice--;
        }

        if (current_thread->timeslice == 0)
        {
            sched_yield();
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

    if (old)
    {
        if (old->state == THREAD_RUNNING)
        {
            sched_enqueue(old);
            old->state = THREAD_READY;
        }
    }
    log_debug(MODULE, "here4");
    // spinlock_release(&schedule_lock);
    // vaddr_t stack_pointer = ctx_arch_get_sp(&current_thread->ctx);
    // page_table_t table;
    // mmu_arch_current_table(&table);
    log_debug(MODULE, "here5");

    if (current_thread->ctx.frame.regs != NULL)
    {
        // ctx_dump(&current_thread->ctx);
    }

    log_debug(MODULE, "here6 switching to 0x%lx", current_thread->kernel_stack);
    // ivt_dump_frame(current_thread->ctx.frame.regs);
    cpu_arch_set_kernel_stack(cpu_arch_get_current(), (vaddr_t)current_thread->kernel_stack);
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
    // log_debug(MODULE, "cpu = %p\n", cpu);
    if (cpu == NULL)
    {
        irq_arch_eoi(0);
        return RETURN_FAILED;
    }
    // if (cpu->current == NULL)
    // {
    //     irq_arch_eoi(0);
    //     return RETURN_FAILED;
    // }

    thread_t *next = sched_next();
    // log_debug(MODULE, "next = %p\n", next);
    if (next == NULL)
    {
        sched_print_thread_info(current_thread);
        current_thread->timeslice = current_thread->timeslice_reset;
        irq_arch_eoi(0);
        return RETURN_GOOD;
    }
    if (next == current_thread)
    {
        fprintf(VFS_FD_DEBUG, "========== SAME ==========\n");
        sched_print_thread_info(current_thread);
        current_thread->timeslice = current_thread->timeslice_reset; // reload
        irq_arch_eoi(0);
        return RETURN_GOOD;
    }

    if (next->proc && next->proc->state == PROC_STATE_SUSPENDED)
    {
        fprintf(VFS_FD_DEBUG, "========== proc is suspended ==========\n");
        current_thread->timeslice = current_thread->timeslice_reset; // reload
        irq_arch_eoi(0);
        return RETURN_GOOD;
    }

    if (regs != NULL && current_thread != NULL)
    {
        // fprintf(VFS_FD_DEBUG, "new frame\n");
        current_thread->kernel_stack = (vaddr_t)regs;
        // ctx_arch_set_sp(&current_thread->ctx, (vaddr_t)regs);
        // ivt_dump_frame(regs);
    }
    else
    {
        // sched_thread_info();
    }
    if (current_thread != NULL)
    {
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
                mmu_arch_map_kernel(next->proc->page_dir);
                mmu_arch_load_table(next->proc->page_dir);
            }
            log_debug(MODULE, "here3");
        }
        current_thread->last_cpu = cpu;
    }

    cpu_arch_set_kernel_stack(cpu, (vaddr_t)next->kernel_stack);

    schedule_switch(next);
    return RETURN_GOOD;
}

void sched_thread_exit()
{
    log_debug(MODULE, "marking t%u as dead", current_thread->tid);
    current_thread->state = THREAD_REMAINS;
    log_debug(MODULE, "find new");
    schedule(NULL); // never returns
}

void sched_init(thread_t *main_thread)
{
    memcpy(main_thread->name, "MAIN\0", 4);
    log_info(MODULE, "setting T%u (%s) as the main thread", main_thread->tid, main_thread->name);
    main_thread->state = THREAD_READY;
    cpu_t *cpu = cpu_arch_get_current();
    // cpu->current = main_thread;
    main_thread->cpu_affinity = cpu;
    sched_add(main_thread);
    // queue[0] = main_thread;
    // queue_size = 1;
    // queue_head = 0;
    // total_threads++;
    ivt_arch_set_handler(0x7F, schedule);
    for (size_t i = 0; i < smp_arch_cpu_count(); i++)
    {
        cpu = cpu_arch_get(i);
        timer_priv_t *p = (timer_priv_t *)cpu->lapic_timer_dev->priv;
        p->set_periodic(cpu->lapic_timer_dev, SCHEDULER_TICK_NS, scheduler_tick);
    }
    // timer_set_oneshot(SCHEDULER_TICK_NS, scheduler_tick);
}
