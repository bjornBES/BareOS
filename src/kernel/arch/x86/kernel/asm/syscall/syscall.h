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
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
#endif
    uint64_t di;
    uint64_t si;
    uint64_t bp;
    uint64_t bx;
    uint64_t dx;
    uint64_t cx;
    uint64_t ax;

    uint64_t pc;
    uint64_t flags;
    uint64_t sp;
} __attribute__((packed)) syscall_frame_t;

void syscall_per_cpu_init();
