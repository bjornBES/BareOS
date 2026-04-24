/*
 * File: process.h
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "arch/x86/isr.h"
#include "memory/paging/paging.h"
#include "syscall/pledge.h"

#include <stdint.h>
#include <stddef.h>

typedef uint64_t pid;

typedef struct
{
    #ifdef __i686__
    uint32_t pc;
    uint32_t bp;
    uint32_t sp;
    uint32_t di;
    uint32_t si;
    uint32_t bx;
    uint32_t dx;
    uint32_t cx;
    uint32_t ax;
    #else
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t pc;
    uint64_t bp;
    uint64_t sp;
    uint64_t di;
    uint64_t si;
    uint64_t bx;
    uint64_t dx;
    uint64_t cx;
    uint64_t ax;
    #endif
    uint16_t ds, cs;
    uint16_t ss, es;
    uint16_t gs, fs;
    #ifdef __i686__
    uint32_t flags;
    #else
    uint64_t flags;
    #endif
} process_regs;

struct __process_t;

// the address this is a process
typedef struct __task_ladder_t
{
    // the next one on the level aka going down i+1
    struct __task_ladder_t *next;
    // the last one on the level aka going up i-1
    struct __task_ladder_t *priv;
} task_ladder;


typedef struct __process_t
{
    // the last one in the chain
    struct __process_t *parent;

    // child, just the child of this process
    struct __process_t *child;

    // siblings am i right?
    task_ladder ladder;
    int ladder_count;
    
    pid pid;
    virt_addr entry;

    // === paging ===

    struct paging_page_t page_dir_virt;
    struct paging_page_t page_dir_phys;

    // === stack ===
    // fixed virtual address of stack top (USER_STACK_TOP)
    virt_addr stack_top;

    // current mapped size in bytes, grows by PAGE_SIZE
    size_t stack_size;

    virt_addr load_base;
    virt_addr load_end;
    phys_addr phys_load_base;

    // pledge
    bool pledged;
    pledge_flags_t pledges;

    // === later things ===

    process_regs regs;
} process;

extern process *current_process;

void process_unexec_process(pid pid);
int process_exec(char *path, char *argv[]);
int process_kill(pid pid, int sig);
void process_init();
