/*
 * File: process.h
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 23:29:01
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "process_types.h"

#include "process_config.h"

#include "kernel/registers.h"
#include "memory/paging/paging.h"
#include "syscall/pledge.h"
#include "signal_type.h"
#include "threading/thread_type.h"
#include "kernel.h"

#include <stdint.h>
#include <stddef.h>

#define SECTION_FLAG_ALLOCATED  (1 << 0)
#define SECTION_FLAG_EXEC       (1 << 1)
#define SECTION_FLAG_WRITE      (1 << 2)
#define SECTION_FLAG_USER       (1 << 3)

extern process_t *current_process;

void kernel_init_process(const char *path, char *argv[], char *envp[]);
int process_unexec_process(process_t *proc);
int process_kill(pid_t pid, int sig);
process_t* process_get_current();
void process_init();
