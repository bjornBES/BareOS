/*
 * File: ivt.h
 * File Created: 01 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct intr_frame
{
    reg_t ds;
#ifdef __x86_64__
    reg_t r15;
    reg_t r14;
    reg_t r13;
    reg_t r12;
    reg_t r11;
    reg_t r10;
    reg_t r9;
    reg_t r8;
#endif
    reg_t di;
    reg_t si;
    reg_t bp;
    reg_t bx;
    reg_t dx;
    reg_t cx;
    reg_t ax;

    reg_t interrupt;
    reg_t error;

    vaddr_t pc;
    reg_t cs;
    
    reg_t flags;

    vaddr_t sp;
    reg_t ss;

} __attribute__((packed)) intr_frame_t;
