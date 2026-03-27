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
#include "arch/x86/paging/paging.h"

#include <stdint.h>
#include <stddef.h>

typedef uint64_t pid;

typedef struct
{
    union
    {
        struct
        {
            uint64_t rip;
            uint64_t rdi;
            uint64_t rsi;
            uint64_t rbp;
            uint64_t rsp;
            uint64_t rbx;
            uint64_t rdx;
            uint64_t rcx;
            uint64_t rax;
        };
        struct
        {
            uint32_t eiph, eip;
            uint32_t edih, edi;
            uint32_t esih, esi;
            uint32_t ebph, ebp;
            uint32_t esph, esp;
            uint32_t ebxh, ebx;
            uint32_t edxh, edx;
            uint32_t ecxh, ecx;
            uint32_t eaxh, eax;
        } Reg32;
    };
    uint16_t ds, cs;
    uint16_t ss, es;
    uint16_t gs, fs;
    uint32_t eflags;
} process_regs;

struct __process_t;

typedef struct __process_t
{
    pid pid;
    uint64_t entry;

    // === paging ===

    struct paging_page_t *page_dir_virt;
    struct paging_page_t *page_dir_phys;

    // === stack ===
    // fixed virtual address of stack top (USER_STACK_TOP)
    uint64_t stack_top;

    // current mapped size in bytes, grows by PAGE_SIZE
    size_t stack_size;

    uint64_t load_base;
    uint64_t load_end;

    // === later things ===

    process_regs regs;
} process;

int exec(char *path, char *argv[]);
void process_init();
