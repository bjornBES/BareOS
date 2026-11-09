/*
 * File: processt.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "pidt.h"

#include "vfs/vfs_table.h"

#include "thread/threadt.h"

#include "loader/loadert.h"

#include "mm/vma/vmat.h"
#include "mm/mmu.h"

#include <config.h>
#include <types.h>
#include <pledge/pledget.h>

typedef enum process_state
{
    PROC_STATE_RUNNING,
    PROC_STATE_READY,
    PROC_STATE_SUSPENDED,
    PROC_STATE_SLEEP,
    PROC_STATE_ZOMBIE,
} process_state_t;

typedef union process_exit_code
{
    struct
    {
        uint8_t res1;
        uint8_t exit_code2;
        uint8_t exit_code1;
        uint8_t signal;
    };

    uint32_t raw;
} process_exit_code_t;

typedef struct thread thread_t;

typedef struct process
{
    LIST_ENTRY_THIS();

    spinlock_t pdb_lock;

    struct process *parent;

    // child, just the child of this process
    list_t children;

    // process info
    pid_block_t *pid_blk;
    pid_t pid;
    pid_t pgid;

    char path[CONFIG_MAX_PATH_LENGTH];
    char volume[CONFIG_MAX_VOLUME_LENGTH];

    uid_t user;
    process_state_t state;

    // memory
    vma_memory_t *vma;

    // pledge
    bool pledged;
    pledge_flags_t pledge_mask;

    // sections
    loader_data_t loader_data;

    // exit state
    process_exit_code_t exit_code;

    // threads
    list_t threads;
    thread_t *main_thread;

    // process
    fd_table_t fd_table;
} process_t;

#define to_process(n) container_of(n, process_t, LIST_ENTRY_NAME)
