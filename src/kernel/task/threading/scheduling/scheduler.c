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

#include "algorithms/mlfq.h"

#include "task/process.h"
#include "debug/debug.h"

#include "math.h"
#include <lists/list.h>
#include <types.h>

#define MODULE            "SCHEDULER"

#define SCHEDULER_TICK_NS 1000000ull // 1ms

#define current_thread    (cpu_arch_get_current()->current)

const sched_class_t *sched_algorithms[1];
const sched_class_t *active_sched_class;

typedef struct
{
    spinlock_t lock;
    sched_class_t *sched_class;
    void *threads; // intrusive list_node_t, same pattern as your other lists
    // int count;     // cheap "is it worth checking" hint before taking the lock
} global_runq_t;

typedef struct
{
    spinlock_t lock;
    list_t threads;
    int count; // cheap "is it worth checking" hint before taking the lock
} global_sleepq_t;

global_runq_t sched_runq;    // one instance, kernel-wide
global_sleepq_t sleep_queue; // one instance, kernel-wide

uint32_t total_threads = 0;

static thread_t *queue[MAX_THREADS] = {0};
// static uint32_t queue_size = 0;

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

static void sched_thread_reap(thread_t *t)
{
    ENTER_FUNC(MODULE, "%p", t);
    sched_print_thread_info(t);
    t->state = THREAD_DEAD;

    THREAD_EXIT(t);
}

static inline void sched_algorithm_enqueue(sched_class_t *class, void *runq_data, thread_t *t)
{
    class->ops.enqueue(runq_data, t);
}

void sched_enqueue(thread_t *t)
{
    log_debug(MODULE, "enqueuing thread %u", t->tid);
    cpu_t *target = NULL;

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
        sched_algorithm_enqueue(target->sched_class, target->runq_data, t);
        // list_push_tail(&target->local_runq, &t->node);
        // target->local_count++;
        spinlock_release(&target->local_runq_lock);

        if (target != cpu_arch_get_current())
        {
            smp_arch_send_ipi(target->apic_id, IPI_RESCHEDULE_VECTOR);
        }
    }
    else
    {
        log_debug(MODULE, "in global");
        spinlock_acquire(&sched_runq.lock);
        sched_algorithm_enqueue(sched_runq.sched_class, sched_runq.threads, t);
        // list_push_tail(&sched_runq.threads, &t->node);
        // sched_runq.count++;
        spinlock_release(&sched_runq.lock);
    }
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
    t->state = THREAD_REMAINS;
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
    sleep_queue.count++;
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
        sleep_queue.count--;
        spinlock_release(&sleep_queue.lock);

        t->wake_time = 0;
        t->state = THREAD_READY;
        sched_enqueue(t);                    // goes to last_cpu/affinity or global, as normal

        spinlock_acquire(&sleep_queue.lock); // re-lock to continue checking the new head
    }
    spinlock_release(&sleep_queue.lock);
}

void sched_block(block_queue_t *queue)
{
    thread_t *self = current_thread;
    self->state = THREAD_BLOCKED;

    lock(&queue->lock);
    list_push_tail(&queue->queue, &self->node);
    unlock(&queue->lock);

    sched_yield();
}

void sched_wake_one(block_queue_t *queue)
{
    lock(&queue->lock);
    list_node_t *n = list_pop_head(&queue->queue);
    unlock(&queue->lock);

    if (!n)
    {
        return;
    }

    thread_t *t = container_of(n, thread_t, node);
    if (t->state == THREAD_REMAINS)
    {
        sched_thread_reap(t);
        return;
    }
    t->state = THREAD_READY;
    sched_enqueue(t); // back through the algorithm's enqueue(), on whatever CPU fits
}

void sched_wake_all(block_queue_t *queue)
{
    lock(&queue->lock);
    list_node_t *n;
    list_t drained;
    list_init(&drained);
    while ((n = list_pop_head(&queue->queue)) != NULL)
    {
        list_push_tail(&drained, n); // drain fully under the lock first
    }
    unlock(&queue->lock);

    list_node_t *n2;
    while ((n2 = list_pop_head(&drained)) != NULL)
    {
        thread_t *t = container_of(n2, thread_t, node);
        if (t->state == THREAD_REMAINS)
        {
            sched_thread_reap(t);
            continue;
        }
        t->state = THREAD_READY;
        sched_enqueue(t); // unlocked now, safe to call sched_enqueue's own locking
    }
}

// void sched_block(thread_t *t)
// {
//     // remove from run queue
//     for (uint32_t i = 0; i < MAX_THREADS; i++)
//     {
//         if (queue[i] == t)
//         {
//             queue[i] = NULL;
//             queue_size--;
//             break;
//         }
//     }
//     // add to blocked queue
//     for (uint32_t i = 0; i < MAX_THREADS; i++)
//     {
//         if (blocked_queue[i] == NULL)
//         {
//             blocked_queue[i] = t;
//             blocked_queue_size++;
//             break;
//         }
//     }
//     t->state = THREAD_BLOCKED;

//     sched_thread_info();
// }

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

cpu_t *pick_steal_victim(cpu_t *me)
{
    cpu_t *busiest = NULL;
    int max_count = 0;

    for (size_t i = 0; i < smp_arch_cpu_count(); i++)
    {
        cpu_t *other = cpu_arch_get(i);
        if (other == me)
        {
            continue;
        }
        int other_count = other->sched_class->ops.thread_count(other->runq_data);
        if (other_count > max_count)
        {
            max_count = other_count;
            busiest = other;
        }
    }
    return (max_count > 1) ? busiest : NULL; // don't steal down to 0, leave victim something
}

static thread_t *sched_next()
{
    cpu_t *me = cpu_arch_get_current();

    // 1. own local queue
    spinlock_acquire(&me->local_runq_lock);
    thread_t *thread = me->sched_class->ops.pick_next(me->runq_data);
    if (thread != NULL)
    {
        // log_debug(MODULE, "thread from cpu %u", me->cpu_id);
        sched_print_thread_info(thread);
        spinlock_release(&me->local_runq_lock);
        return thread;
    }
    spinlock_release(&me->local_runq_lock);
    // log_debug(MODULE, "next");

    // 2. global queue
    spinlock_acquire(&sched_runq.lock);
    thread = sched_runq.sched_class->ops.pick_next(sched_runq.threads);
    if (thread != NULL)
    {
        sched_print_thread_info(thread);
        spinlock_release(&sched_runq.lock);
        return thread;
    }
    spinlock_release(&sched_runq.lock);
    // log_debug(MODULE, "next");

    cpu_t *other = pick_steal_victim(me);
    if (other != NULL)
    {
        cpu_t *first = (me->cpu_id < other->cpu_id) ? me : other;
        cpu_t *second = (me->cpu_id < other->cpu_id) ? other : me;
        spinlock_acquire(&first->local_runq_lock);
        spinlock_acquire(&second->local_runq_lock);

        thread = other->sched_class->ops.steal_one(other->runq_data);

        spinlock_release(&second->local_runq_lock);
        spinlock_release(&first->local_runq_lock);

        if (thread != NULL)
        {
            return thread;
        }
    }

    // log_debug(MODULE, "NULL");
    return NULL; // caller goes idle (hlt)
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
    cpu_t *cpu = cpu_arch_get_current();
    cpu->sched_class->ops.yield(cpu->runq_data, cpu->current);
    return scheduler_arch_yield();
}

SYSCALL_DEFINE0(sched_yield);

void scheduler_tick(device_t *dev)
{
    fprintf(VFS_FD_DEBUG, "scheduler_tick\n");
    scheduler_wakeup_check();

    if (current_thread == NULL)
    {
        return;
    }
    uint32_t level_before = current_thread->priority;
    cpu_t *cpu = cpu_arch_get_current();
    cpu->sched_class->ops.tick(cpu->runq_data, cpu->current);

    // rearm — next tick or next wakeup whichever sooner
    // uint64_t next = SCHEDULER_TICK_NS;
    // timer_set_oneshot(next, scheduler_tick);

    if (current_thread->state == THREAD_RUNNING)
    {
        if (current_thread->timeslice == 0 || current_thread->priority != level_before)
        {
            sched_yield();
            return;
        }
    }
}

__attribute__((noreturn)) void schedule_switch(thread_t *next)
{
    // thread_t *old = current_thread;
    current_thread = next;
    current_thread->state = THREAD_RUNNING;
    current_thread->timeslice = current_thread->timeslice_reset;

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
    cpu_t *cpu = cpu_arch_get_current();
    if (current_thread)
    {
        current_thread->last_cpu = cpu;
        if (current_thread->state == THREAD_REMAINS)
        {
            log_debug(MODULE, "thread (%u) is gonna die soon", current_thread->tid);
            sched_enqueue(current_thread);
        }
        else if (current_thread->state == THREAD_RUNNING)
        {
            sched_enqueue(current_thread);
            current_thread->state = THREAD_READY;
        }
    }
    // spinlock_acquire(&schedule_lock);
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
try_again:
    thread_t *next = sched_next();
    // log_debug(MODULE, "next = %p\n", next);
    if (next == NULL)
    {
        irq_arch_eoi(0);
        return RETURN_GOOD;
    }
    if (next->state == THREAD_REMAINS && next != current_thread)
    {
        sched_thread_reap(next);
        goto try_again;
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
    active_sched_class = mlfq_get_class();
    sched_runq.sched_class = (sched_class_t *)active_sched_class;
    sched_runq.threads = sched_runq.sched_class->ops.init(sched_runq.threads);

    memcpy(main_thread->name, "MAIN\0", 4);
    log_info(MODULE, "setting T%u (%s) as the main thread", main_thread->tid, main_thread->name);
    main_thread->state = THREAD_READY;
    cpu_t *cpu = cpu_arch_get_current();
    main_thread->cpu_affinity = cpu;

    ivt_arch_set_handler(0x7F, schedule);
    for (size_t i = 0; i < smp_arch_cpu_count(); i++)
    {
        cpu = cpu_arch_get(i);
        timer_priv_t *p = (timer_priv_t *)cpu->lapic_timer_dev->priv;
        p->set_periodic(cpu->lapic_timer_dev, SCHEDULER_TICK_NS, scheduler_tick);

        cpu->sched_class = (sched_class_t *)active_sched_class;
        cpu->runq_data = cpu->sched_class->ops.init(cpu->runq_data);
    }
    sched_add(main_thread);
}
