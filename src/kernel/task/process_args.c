/*
 * File: process_args.c
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process_args.h"

#include "kernel/string.h"
#include "kernel/memory.h"

#define MODULE "PROC-ARG"

process_args_t *process_args_save(process_t *proc, char *argv[], char *envp[])
{
    ENTER_FUNC(MODULE, "%p, %p, %p", proc, argv, envp);
    process_args_t *info = malloc(sizeof(process_args_t));
    int argc = 0;
    int envc = 0;
    size_t str_size = 0;

    for (size_t i = 0; i < MAX_ARGS; i++)
    {
        if (argv[i] == NULL)
        {
            break;
        }
        log_debug(MODULE, "count of %s", argv[i]);
        uint32_t length = strlen(argv[i]) + 1;
        str_size += length;
        info->saved_argv[i] = malloc(length);
        strcpy(info->saved_argv[i], argv[i]);
        argc++;
    }

    if (envp != NULL)
    {
        for (size_t i = 0; i < MAX_ARGS; i++)
        {
            if (envp[i] == NULL)
            {
                break;
            }
            log_debug(MODULE, "count of %s", envp[i]);
            uint32_t length = strlen(envp[i]) + 1;
            str_size += length;
            info->saved_envp[i] = malloc(length);
            strcpy(info->saved_envp[i], envp[i]);
            str_size += length;
            envc++;
        }
    }
    info->argc = argc;
    info->envc = envc;
    info->str_size = str_size;
    return info;
}

int process_args_parse(process_args_t *info, process_t *proc)
{
    ENTER_FUNC(MODULE, "%p, %p", info, proc);
    int argc = info->argc;
    int envc = info->envc;
    size_t str_size = info->str_size;

    char **temp_argv = info->saved_argv;
    char **temp_envp = info->saved_envp;

    reg_t user_stack = (reg_t)proc->vma->start_stack;
    user_stack &= ~0xFull;
    user_stack -= str_size;
    log_debug(MODULE, "user_stack = %p", user_stack);

    reg_t stack_base = user_stack;

    uint64_t table_size = sizeof(uint64_t) * 3;
    table_size += argc * sizeof(uint64_t);
    table_size += envc * sizeof(uint64_t);

    user_stack -= table_size;
    user_stack &= ~0xFull;

    reg_t table_cursor = user_stack;
    *((uint64_t *)table_cursor) = (uint64_t)argc;
    log_debug(MODULE, "argc (%d) @ %p", argc, table_cursor);
    table_cursor += sizeof(uint64_t);

    log_debug(MODULE, "argc = %u", argc);
    for (size_t i = 0; i < argc; i++)
    {
        size_t length = strlen(temp_argv[i]) + 1;
        memcpy((void *)stack_base, temp_argv[i], length);
        *((uint32_64 *)table_cursor) = (uint32_64)stack_base;
        log_debug(MODULE, "argv[%d] = %s @ %p", i, temp_argv[i], table_cursor);
        stack_base += length;
        table_cursor += sizeof(uint64_t);
        free(temp_argv[i]);
    }

    *((uint64_t *)table_cursor) = 0;
    log_debug(MODULE, "0 @ %p", table_cursor);
    table_cursor += sizeof(uint64_t);

    if (info->envc != 0)
    {
        log_debug(MODULE, "envc = %u", envc);
        for (int i = 0; i < envc; i++)
        {
            if (temp_envp[i] == NULL)
            {
                log_warn(MODULE, "envp[%u] is null", i);
                continue;
            }
            size_t length = strlen(temp_envp[i]) + 1;
            memcpy((void *)stack_base, temp_envp[i], length);
            *((uint64_t *)table_cursor) = (uint64_t)stack_base;
            log_debug(MODULE, "envp[%d] = %s @ %p", i, temp_envp[i], table_cursor);
            stack_base += length;
            table_cursor += sizeof(uint64_t);
            free(temp_envp[i]);
        }
    }

    *((uint64_t *)table_cursor) = 0;
    log_debug(MODULE, "0 @ %p", table_cursor);

    proc->vma->start_stack = (vaddr_t)user_stack;
    free(info);

    return argc;
}
