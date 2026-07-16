/*
 * File: ctx.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#include "kernel/asm/threading/ctx/ctx.h"
#include "kernel/asm/ivt/ivt.h"

typedef struct intr_frame intr_frame_t;
typedef struct cpu_ctx cpu_ctx_t;

// initialize a fresh context for a new thread
vaddr_t ctx_arch_init(cpu_ctx_t *ctx, vaddr_t entry, vaddr_t kstack, vaddr_t ustack, vaddr_t arg);

void ctx_dump(cpu_ctx_t *ctx);

// switch from one thread context to another
void ctx_arch_switch(vaddr_t to);

// get/set instruction pointer — for signals, debugging
vaddr_t ctx_arch_get_ip(cpu_ctx_t *ctx);
void ctx_arch_set_ip(cpu_ctx_t *ctx, vaddr_t ip);

// get/set stack pointer
vaddr_t ctx_arch_get_sp(cpu_ctx_t *ctx);
void ctx_arch_set_sp(cpu_ctx_t *ctx, vaddr_t sp);

// get/set return value register — for syscall return
void ctx_arch_set_retval(cpu_ctx_t *ctx, uint64_t val);

// get/set instruction pointer — for signals, debugging
vaddr_t frame_arch_get_ip(intr_frame_t *frame);
void frame_arch_set_ip(intr_frame_t *frame, vaddr_t ip);

// get/set stack pointer
vaddr_t frame_arch_get_sp(intr_frame_t *frame);
void frame_arch_set_sp(intr_frame_t *frame, vaddr_t sp);

// get/set return value register — for syscall return
void frame_arch_set_retval(intr_frame_t *frame, uint64_t val);

void ctx_arch_clone_frame(intr_frame_t *child_frame, intr_frame_t *parent_frame);
vaddr_t ctx_arch_init_fork(cpu_ctx_t *child_ctx, vaddr_t kernel_stack_top, intr_frame_t *child_frame);
