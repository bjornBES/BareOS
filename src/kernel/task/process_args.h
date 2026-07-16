/*
 * File: process_args.h
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "process_types.h"

typedef struct process_args
{
    char *saved_argv[MAX_ARGS];
    char *saved_envp[MAX_ARGS];
    int argc;
    int envc;
    size_t str_size;
} process_args_t;

process_args_t *process_args_save(process_t *proc, char *argv[], char *envp[]);
int process_args_parse(process_args_t *info, process_t *proc);
