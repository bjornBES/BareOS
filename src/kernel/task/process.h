/*
 * File: process.h
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "process_types.h"

#include "process_config.h"

#include "syscall/pledge.h"
#include "signal/signal_type.h"
#include "threading/thread_type.h"
#include "kernel.h"

#include <stdint.h>
#include <stddef.h>

#define SECTION_FLAG_ALLOCATED  (1 << 0)
#define SECTION_FLAG_EXEC       (1 << 1)
#define SECTION_FLAG_WRITE      (1 << 2)
#define SECTION_FLAG_USER       (1 << 3)

int process_exec(char *proc_path, char *argv[], char *envp[], process_t *proc);
int process_unexec_process(process_t *proc);
int process_kill(pid_t pid, int sig);
process_t* process_get_current();
void process_init();
