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

#include "paging/paging.h"
#include "arch/i686/isr.h"

#include <core/arch/i686/bios.h>

#include <stdint.h>
#include <stddef.h>

typedef uint32_t pid;

typedef struct
{
    union
    {
        struct
        {
            uint32_t eip;
            uint32_t edi;
            uint32_t esi;
            uint32_t ebp;
            uint32_t esp;
            uint32_t ebx;
            uint32_t edx;
            uint32_t ecx;
            uint32_t eax;
        } Reg32;
        struct
        {
            uint16_t ip, hip;
            uint16_t di, hdi;
            uint16_t si, hsi;
            uint16_t bp, hbp;
            uint16_t sp, hsp;
            uint16_t bx, hbx;
            uint16_t dx, hdx;
            uint16_t cx, hcx;
            uint16_t ax, hax;
        } Reg16;
        struct
        {
            uint8_t ipl, iph, eip2, eip3;
            uint8_t dil, dih, edi2, edi3;
            uint8_t sil, sih, esi2, esi3;
            uint8_t bpl, bph, ebp2, ebp3;
            uint8_t spl, sph, esp2, esp3;
            uint8_t bl, bh, ebx2, ebx3;
            uint8_t dl, dh, edx2, edx3;
            uint8_t cl, ch, ecx2, ecx3;
            uint8_t al, ah, eax2, eax3;
        } Reg8;
    };
    uint16_t ds, cs;
    uint16_t ss, es;
    uint16_t gs, fs;
    uint32_t eflags;
} process_regs;

typedef struct
{
    pid pid;
    uint32_t entry;

    // === paging ===

    page_directory_entry *page_dir;

    // === stack ===
    // fixed virtual address of stack top (USER_STACK_TOP)
    uint32_t stack_top;

    // current mapped size in bytes, grows by PAGE_SIZE
    size_t stack_size;

    uint32_t load_base;
    uint32_t load_end;

    // === later things ===

    process_regs regs;
} process;

int exec(char *path, char *argv[]);
