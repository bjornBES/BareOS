/*
 * File: process_types.h
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "process_config.h"

#include "signal/signal_type.h"
#include "threading/thread_type.h"
#include "mm/paging/paging_type.h"
#include "mm/vmm/vmm_types.h"

#include <types.h>
#include <pledge_types.h>

struct process;
struct __signal_table_t;

typedef enum process_abi
{
    Undefined,
    ABI_SYSV64,
    ABI_SYSV32,
} process_abi_t;

typedef enum process_state
{
    PROC_STATE_RUNNING,
    PROC_STATE_READY,
    PROC_STATE_SLEEP,
    PROC_STATE_ZOMBIE,
} process_state_t;

typedef enum process_section_type
{
    PROC_TYPE_TEXT,
    PROC_TYPE_DATA,
    PROC_TYPE_RODATA,
    PROC_TYPE_BSS,
    PROC_TYPE_HEAP,
    PROC_TYPE_STACK,
} process_section_type_t;

typedef struct process_section
{
    vaddr_t base;
    size_t size;
    process_section_type_t type;
    uint32_t flags;
} process_section_t;

// the address this is a process
typedef struct task_ladder
{
    // the next one on the level aka going down i+1
    struct task_ladder *next;
    // the last one on the level aka going up i-1
    struct task_ladder *priv;
} task_ladder_t;

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


typedef struct process
{
    // children and siblings
    // siblings am i right?
    task_ladder_t ladder;
    
    // the last one in the chain
    struct process *parent;
    
    // child, just the child of this process
    struct process *child;
    
    // process info
    pid_t pid;
    char path[MAX_PATH_SIZE];
    char volume[MAX_VOLUME_NAME];
    uint16_t abi;
    uid_t user;
    process_state_t state;
    vaddr_t entry;

    pid_t wait_for;

    // memory
    vma_memory_t *vma;

    // paging
    page_table_t *page_dir;

    // pledge
    bool pledged;
    pledge_flags_t pledge_mask;

    // sections
    process_section_t sections[PROCESS_MAX_SECTIONS];
    uint8_t section_count;

    // signals
    signal_pending signal_queue;
    struct __signal_table_t signal_table;
    process_exit_code_t exit_code;

    // threads
    thread_t *threads[MAX_THREADS_PER_PROCESS];
    uint32_t thread_count;

    // === later things ===
} process_t;
