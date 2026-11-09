/*
 * File: pid.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process/processt.h"
#include "process/pid.h"

#include "assert/assert.h"

#include "init.h"

#include "memory.h"

#include <array.h>
#include <types.h>

#define MODULE "pid"

pid_block_t init_pid_block = {
    .allocated = 0,
    .base = 0,
    .hint = 0,
};

pid_block_t *blocks[MAX_BLOCKS];
#define LOCK_NAME pid_lock
DEF_LOCK(LOCK_NAME);

ALWAYS_INLINE INTERNAL void pid_block_lock(pid_block_t *proc)
{
    spinlock_acquire(&proc->block_lock);
}

ALWAYS_INLINE INTERNAL void pid_block_unlock(pid_block_t *proc)
{
    spinlock_release(&proc->block_lock);
}

INTERNAL int32_t find_free_block_index()
{
    WARN_LOCK_UNLOCKED();
    for (size_t i = 0; i < ARRAY_SIZE(blocks); i++)
    {
        if (blocks[i] == NULL)
        {
            return i;
        }
    }
    return -1;
}

ALWAYS_INLINE INTERNAL uint32_t to_block_index(pid_t pid)
{
    return pid / PID_MAX;
}
ALWAYS_INLINE INTERNAL uint32_t to_block_slot(pid_t pid)
{
    return pid % PID_MAX;
} 

INTERNAL status_t pid_block_init(pid_block_t *b, int index)
{
    WARN_LOCK_UNLOCKED();
    b->procs = kzmalloc(sizeof(process_t *) * PID_MAX);
    b->allocated = 0;
    b->hint = 0;
    b->base = index * PID_MAX;
    return KERRNO_SUCCESSES;
}

INTERNAL pid_block_t *pid_allocate_block(int index)
{
    WARN_LOCK_UNLOCKED();
    pid_block_t *block = kzmalloc(sizeof(pid_block_t));
    blocks[index] = block;
    pid_block_init(block, index);
    return block;
}

INTERNAL process_t *pid_block_lookup(pid_t pid)
{
    pid_block_t *b = blocks[to_block_index(pid)];
    if (!b)
    {
        return NULL;
    }
    return b->procs[to_block_slot(pid)];
}

status_t pid_allocate(process_t *proc)
{
    ENTER_FUNC("%p", proc);
    if (init_pid_block.allocated == 0)
    {
        proc->pid_blk = &init_pid_block;
    }
    pid_block_t *current_block = proc->pid_blk;
    trace_debug(MODULE, "current_block = %p", current_block);
    pid_block_lock(current_block);
    
    if (current_block->allocated >= PID_MAX)
    {
        LOCK();
        int blk_index = find_free_block_index();
        pid_block_unlock(current_block);
        current_block = pid_allocate_block(blk_index);
        pid_block_lock(current_block);
        UNLOCK();
    }

    pid_t pid = -2;
    for (uint32_t i = current_block->hint; i < PID_MAX; i++)
    {
        if (i == current_block->hint && pid != -2)
        {
            trace_crit(MODULE, "current block is full");
            KERNEL_PANIC(MODULE, "");
            // TODO when this hits then do something about it.
        }
        if (i >= PID_MAX)
        {
            i = 0;
        }
        pid = -1;
        if (TEST_BIT(i, current_block->bitmap) == 0)
        {
            SET_BIT(i, current_block->bitmap);
            current_block->procs[i] = proc;
            current_block->hint = i;
            current_block->allocated++;
            pid = current_block->base + i;
            trace_debug(MODULE, "pid = %u", pid);
            // trace_debug(MODULE, "bit[%u] = %u", i, TEST_BIT(i, current_block->bitmap));
            break;
        }
    }
    if (pid < 0)
    {
        pid_block_unlock(current_block);
        KERRNO_RETURN(EAGAIN, "PID block is exhausted");
    }
    
    pid_block_unlock(current_block);

    proc->pid = pid;
    return KERRNO_SUCCESSES;
}

status_t pid_init()
{
    ASSERT(MAX_BLOCKS * PID_MAX >= sizeof(pid_t), "MAX_BLOCKS * PID_MAX is not within pid_t");
    LOCK();
    blocks[0] = &init_pid_block;
    pid_block_init(&init_pid_block, 0);
    UNLOCK();
    return KERRNO_SUCCESSES;
}

POSTCORE_INITCALL(pid_init);
