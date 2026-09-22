/*
 * File: sched.c
 * File Created: 19 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sched/sched.h"
#include "sched/sched_algorithm.h"

#include "thread/thread.h"

#include "asm/smp_arch.h"
#include "asm/cpu_arch.h"
#include "asm/irq_arch.h"
#include "asm/vectors_arch.h"
#include "asm/sched_arch.h"
#include "asm/context_arch.h"
#include "asm/frame_arch.h"

#include "ivt/ivt.h"
#include "irq/irq.h"
#include "smp/smp.h"

#include "sync/spinlock.h"

#include "debug/debug.h"

#include "memory.h"

#include <config.h>

#define MODULE "sched"

#define SCHEDULER_TICK_NS 5000000ull // 5ms

#define current_thread    (cpu_arch_get_current()->current)

sched_class_t algorithms_table[CONFIG_MAX_SCHED_ALGORITHMS];

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

uint8_t sched_init_done;

uint32_t total_threads = 0;

// static uint32_t queue_size = 0;

static thread_t *blocked_queue[CONFIG_SCHED_MAX_THREADS] = {0};
static uint32_t blocked_queue_size = 0;

// static uint32_t queue_head = 0;

void scheduler_tick(void *_)
{
    if (sched_init_done == 0)
    {
        return;
    }
    /* // fprintf(VFS_FD_DEBUG, "scheduler_tick\n");
    scheduler_wakeup_check();

    if (current_thread == NULL)
    {
        schedule(NULL);
        return;
    }
    uint32_t level_before = current_thread->priority;
    cpu_t *cpu = cpu_arch_get_current();
    cpu->sched_class->ops.tick(cpu->runq_data, cpu->current);

    if (cpu->need_resched)
    {
        cpu->need_resched = false;
        sched_yield();
        return;
    }

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
    } */
}

INTERNAL INLINE cpu_t *pick_steal_victim(cpu_t *me)
{
    // ENTER_FUNC(MODULE, "%p", me);
    cpu_t *busiest = NULL;
    int max_count = 0;

    for (size_t i = 0; i < cpu_get_count(); i++)
    {
        cpu_entry_t *entry = cpu_get_entry(i);
        cpu_t *other = entry->cpu;
        if (other == me)
        {
            continue;
        }
        if (other == NULL)
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
INTERNAL INLINE thread_t *sched_next()
{
    cpu_t *me = cpu_arch_get_current();

    // 1. own local queue
    spinlock_acquire(&me->local_runq_lock);
    thread_t *thread = me->sched_class->ops.pick_next(me->runq_data);
    if (thread != NULL)
    {
        // trace_debug(MODULE, "thread from cpu %u", me->logical_id);
        // sched_print_thread_info(thread);
        spinlock_release(&me->local_runq_lock);
        return thread;
    }
    spinlock_release(&me->local_runq_lock);
    // trace_debug(MODULE, "next");

    // 2. global queue
    spinlock_acquire(&sched_runq.lock);
    thread = sched_runq.sched_class->ops.pick_next(sched_runq.threads);
    if (thread != NULL)
    {
        // sched_print_thread_info(thread);
        spinlock_release(&sched_runq.lock);
        return thread;
    }
    spinlock_release(&sched_runq.lock);
    // trace_debug(MODULE, "next");

    cpu_t *other = pick_steal_victim(me);
    if (other != NULL)
    {
        cpu_t *first = (me->logical_id < other->logical_id) ? me : other;
        cpu_t *second = (me->logical_id < other->logical_id) ? other : me;
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

    // trace_debug(MODULE, "NULL");
    // sched_thread_info();
    return me->idle; // caller goes idle (hlt)
}

extern void hexdump(void *ptr, size_t len, size_t size);

INTERNAL NORETURN void schedule_switch(thread_t *next)
{
    current_thread = next;
    current_thread->state = THREAD_RUNNING;
    current_thread->timeslice = current_thread->timeslice_reset;
    current_thread->in_queue = false;

    // log_debug(MODULE, "here4");
    // spinlock_release(&schedule_lock);
    // vaddr_t stack_pointer = ctx_arch_get_sp(&current_thread->ctx);
    // page_table_t table;
    // mmu_arch_current_table(&table);
    // log_debug(MODULE, "here5");

    cpu_arch_set_kernel_stack(cpu_arch_get_current(), (vaddr_t)current_thread->kstack);
    if (!current_thread->is_idle_thread)
    {
        // log(MODULE, "switching to tid %u 0x%lx\n", current_thread->tid, current_thread->kernel_stack);
        // irq_arch_disable();
        // frame_arch_dump_frame(current_thread->ctx.regs);
    }
    irq_eoi(0);
    context_arch_switch(current_thread->ctx.sp);

    log_debug(MODULE, "something is wrong");
    KERNEL_PANIC(MODULE, "something is wrong");

    // loop
    for (;;);
}

spinlock_t schedule_lock = {0};

status_t schedule(intr_frame_t *regs)
{
    spinlock_acquire(&schedule_lock);
    if (sched_init_done == 0)
    {
        spinlock_release(&schedule_lock);
        irq_eoi(regs->interrupt);
        return KERRNO_SUCCESSES;
    }
    if (current_thread != NULL && !current_thread->is_idle_thread)
    {
        trace_info(MODULE, "enter schedule(%p)\n", regs);
    }
    cpu_t *cpu = THISCPU();

    if (cpu->current)
    {
        current_thread->last_cpu = cpu;
        if (current_thread->is_idle_thread)
        {
            current_thread->state = THREAD_READY;
        }
        else if (current_thread->state == THREAD_BLOCKED)
        {
            ;
        }
        else if (current_thread->state == THREAD_SLEEP)
        {
            ;
        }
        else if (current_thread->state == THREAD_REMAINS)
        {
        }
        else if (current_thread->state == THREAD_RUNNING)
        {
            current_thread->state = THREAD_READY;
            sched_add(current_thread);
        }
        else if (current_thread->state == THREAD_READY)
        {
            sched_add(current_thread);
        }
        else
        {
            trace_debug(MODULE, "don't know what to do with %u", current_thread->tid);
            // sched_print_thread_info(current_thread);
        }
    }
    // trace_debug(MODULE, "finding next now");
    // if (cpu->current == NULL)
    // {
    //     irq_eoi(0);
    //     return RETURN_FAILED;
    // }
try_again:
    thread_t *next = sched_next();
    if (!next->is_idle_thread)
    {
        // log(MODULE, "next = %p tid = %u\n", next, next->tid);
        // irq_arch_disable();
    }
    // sched_print_thread_info(next);
    if (next == NULL)
    {
        KERNEL_PANIC(MODULE, "sched next gave a NULL thread");
    }
    if (next->state == THREAD_REMAINS)
    {
        if (next != current_thread)
        {
            FUNC_NOT_IMPLEMENTED();
            // sched_thread_reap(next);
        }
        sched_add(next);
        goto try_again;
    }
    if (!next->in_queue && next->state != THREAD_READY)
    {
        goto try_again;
    }
    if (next->state != THREAD_READY)
    {
        trace_debug(MODULE, "thread not ready");
        // goto try_again;
    }

/*     if (next->proc && next->proc->state == PROC_STATE_SUSPENDED)
    {
        fprintf(VFS_FD_DEBUG, "========== proc is suspended ==========\n");
        irq_arch_disable();
        current_thread->timeslice = current_thread->timeslice_reset; // reload
        irq_eoi(0);
        // spinlock_release(&schedule_lock);
        return RETURN_GOOD;
    } */
    if (current_thread && current_thread->state == THREAD_REMAINS)
    {
        trace_debug(MODULE, "thread (%u) is gonna die soon", current_thread->tid);
        sched_add(current_thread);
    }

    if (regs != NULL && current_thread != NULL)
    {
        current_thread->ctx.regs = regs;
        if (!current_thread->is_idle_thread)
        {
        }
    }

/*     if (current_thread != NULL)
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
            // trace_debug(MODULE, "here3");
        }
        current_thread->last_cpu = cpu;
    } */

    spinlock_release(&schedule_lock);
    schedule_switch(next);
    return KERRNO_SUCCESSES;
}

static INTERNAL ALWAYS_INLINE void sched_algorithm_enqueue(sched_class_t *class, void *runq_data, thread_t *t)
{
    if (t->in_queue)
    {
        trace_warn(MODULE, "thread %u is already in queue", t->tid);
        return;
    }
    class->ops.enqueue(runq_data, t);
}

status_t sched_add(thread_t *t)
{
    trace_debug(MODULE, "adding thread %u", t->tid);
    if (t->is_idle_thread)
    {
        trace_debug(MODULE, "ignore %u", t->tid);
        return 1;
    }
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
        trace_debug(MODULE, "in cpu %u", target->arch_id);
        spinlock_acquire(&target->local_runq_lock);
        sched_algorithm_enqueue(target->sched_class, target->runq_data, t);
        // list_push_tail(&target->local_runq, &t->node);
        // target->local_count++;
        total_threads++;
        spinlock_release(&target->local_runq_lock);

        if (target != THISCPU())
        {
            if (target->current->tid != t->tid)
            {
                smp_arch_send_ipi(target->arch_id, IPI_RESCHEDULE_VECTOR);
            }
        }
    }
    else
    {
        trace_debug(MODULE, "in global");
        spinlock_acquire(&sched_runq.lock);
        sched_algorithm_enqueue(sched_runq.sched_class, sched_runq.threads, t);
        total_threads++;
        // list_push_tail(&sched_runq.threads, &t->node);
        // sched_runq.count++;
        spinlock_release(&sched_runq.lock);
    }
    return KERRNO_SUCCESSES;
}

status_t sched_has_work(cpu_t *cpu)
{
    spinlock_acquire(&sched_runq.lock);
    if (sched_runq.sched_class->ops.thread_count(sched_runq.threads) > 0)
    {
        // sched_thread_info();
        spinlock_release(&sched_runq.lock);
        return KERRNO_SUCCESSES;
    }
    spinlock_release(&sched_runq.lock);
    spinlock_acquire(&cpu->local_runq_lock);
    if (cpu->sched_class->ops.thread_count(cpu->runq_data) > 0)
    {
        // sched_thread_info();
        spinlock_release(&cpu->local_runq_lock);
        return KERRNO_SUCCESSES;
    }
    spinlock_release(&cpu->local_runq_lock);
    return 1;
}

status_t sched_yield()
{
    // sched_thread_info();
    cpu_t *cpu = cpu_arch_get_current();
    if (cpu == NULL)
    {
        return KERRNO_SUCCESSES;
    }
    if (cpu->sched_class == NULL || cpu->runq_data == NULL)
    {
        return KERRNO_SUCCESSES;
    }
    // ENTER_FUNC(MODULE, "", "");
    cpu->sched_class->ops.yield(cpu->runq_data, cpu->current);
    scheduler_arch_yield();
    KERNEL_PANIC(MODULE, "thread didn't yield");
    while (true)
    {
        ;
    }
}

NORETURN void sched_thread_exit(uintptr_t ret)
{
    trace_debug(MODULE, "marking t%u as dead", current_thread->tid);
    current_thread->return_value = ret;
    current_thread->state = THREAD_REMAINS;
    schedule(NULL); // never returns

    KERNEL_PANIC(MODULE, "thread didn't get scheduled again");
    while (true)
    {
        ;
    }
}

status_t sched_init(thread_t *main_thread)
{
    sched_init_done = 0;
    int active_sched_index = 0;
    if (sched_algorithm_get_algorithm(algorithms_table, &active_sched_index) != KERRNO_SUCCESSES)
    {
        KERRNO_NO_RETURN(KERRNO_NOT_ALLOWED, "Can't get an algorithm for the scheduler");
        KERNEL_PANIC(MODULE, "Can't get an algorithm");
    }
    active_sched_class = &algorithms_table[active_sched_index];
    sched_runq.sched_class = (sched_class_t *)active_sched_class;
    sched_runq.threads = sched_runq.sched_class->ops.init(sched_runq.threads);
    sleep_queue.count = 0;
    list_init(&sleep_queue.threads);

    memcpy(main_thread->name, "MAIN\0", 4);
    trace_info(MODULE, "setting T%u (%s) as the main thread", main_thread->tid, main_thread->name);
    
    thread_set_priority(main_thread, PRIORITY_LOW);
    
    cpu_t *cpu = cpu_arch_get_current();
    main_thread->state = THREAD_READY;
    main_thread->cpu_affinity = cpu;
    main_thread->is_main_thread = true;

    ivt_set_handler(SCHED_SCHEDULE, schedule);
    // trace_debug(MODULE, "here 0");
    periodic_function_args_t *args = kmalloc(sizeof(periodic_function_args_t));
    for (size_t i = cpu_get_count(); i != -1; i--)
    {
        cpu_entry_t *entry = cpu_get_entry(i);
        if (entry == NULL)
        {
            continue;
        }
        cpu_t *other_cpu = entry->cpu;
        trace_debug(MODULE, "here 1 cpu = %p", other_cpu);
        trace_debug(MODULE, "here 2 args = %p", args);
        args->cb = scheduler_tick;
        args->source = other_cpu->cpu_timer_dev;
        args->ns = SCHEDULER_TICK_NS;
        trace_debug(MODULE, "set periodic on cpu %u", entry->arch_id);
        if (other_cpu == cpu)
        {
            cpu->cpu_timer_dev->arm_periodic(cpu->cpu_timer_dev, SCHEDULER_TICK_NS, scheduler_tick);
        }
        else
        {
            smp_call_function(i, timer_set_device_periodic_wrapper, (uintptr_t)args);
        }

        while (other_cpu->func_pending != NULL)
        {
        }

        extern uintptr_t THREAD_CALL_FUNC(smp_arch_idle_thread)(uintptr_t _);
        other_cpu->idle = thread_create_kernel(THREAD_CALL_FUNC(smp_arch_idle_thread), 0, 0);
        thread_set_priority(other_cpu->idle, PRIORITY_MIDDLE_SHORT);
        other_cpu->idle->is_idle_thread = true;

        other_cpu->sched_class = (sched_class_t *)active_sched_class;
        other_cpu->runq_data = other_cpu->sched_class->ops.init(other_cpu->runq_data);
    }

    kfree(args);
    sched_add(main_thread);

    sched_init_done = 1;
    return KERRNO_SUCCESSES;
}
