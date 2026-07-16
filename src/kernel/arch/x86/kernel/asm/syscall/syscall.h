/*
 * File: syscall.h
 * File Created: 06 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once


typedef struct syscall_frame
{
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

    vaddr_t pc;
    reg_t flags;
    vaddr_t sp;
} __attribute__((packed)) syscall_frame_t;

void syscall_per_cpu_init();
