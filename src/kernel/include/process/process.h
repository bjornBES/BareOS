/*
 * File: process.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "processt.h"

status_t proc_allocate(process_t *parent, process_t **out_proc);

status_t proc_add(process_t *parent, process_t *child);

status_t proc_spawn(process_t *parent, process_t *child);

status_t proc_vfork(process_t *parent, process_t *child);
status_t proc_exec(process_t *proc, const char *path, char *argv[], char *envp[]);
status_t proc_kernel_execve(process_t *proc, const char *path, char *argv[], char *envp[], thread_t **out_thread);
status_t proc_add_thread(process_t *proc, thread_t *t);
status_t proc_run(process_t *proc);
