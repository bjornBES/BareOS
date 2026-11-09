/*
 * File: exec.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process/process.h"

#include "asm/mmu_arch.h"
#include "asm/context_arch.h"
#include "asm/cpu_arch.h"

#include "mm/vma/vma.h"

#include "thread/thread.h"

#include "vfs/path.h"
#include "vfs/vfs.h"

#include "loader/loader.h"

#include "kerrno.h"

#include "memory.h"
#include "string.h"

#define MODULE "process-exec"

typedef struct process_args
{
    char *saved_argv[CONFIG_EXEC_MAX_ARGS];
    char *saved_envp[CONFIG_EXEC_MAX_ARGS];
    int argc;
    int envc;
    size_t str_size;
} process_args_t;

process_args_t *process_args_save(process_t *proc, char *argv[], char *envp[])
{
    ENTER_FUNC("%p, %p, %p", proc, argv, envp);
    process_args_t *info = kzmalloc(sizeof(process_args_t));
    int argc = 0;
    int envc = 0;
    size_t str_size = 0;

    {
        uint32_t length = strlen(proc->path) + 1;
        str_size += length;
        info->saved_argv[0] = kzmalloc(length);
        strcpy(info->saved_argv[0], proc->path);
        argc++;
    }

    if (argv != NULL)
    {
        for (size_t i = 0; i < CONFIG_EXEC_MAX_ARGS; i++)
        {
            if (argv[i] == NULL)
            {
                break;
            }
            trace_debug(MODULE, "count of %s", argv[i]);
            uint32_t length = strlen(argv[i]) + 1;
            str_size += length;
            info->saved_argv[i + 1] = kzmalloc(length);
            strcpy(info->saved_argv[i + 1], argv[i]);
            argc++;
        }
    }

    if (envp != NULL)
    {
        for (size_t i = 0; i < CONFIG_EXEC_MAX_ARGS; i++)
        {
            if (envp[i] == NULL)
            {
                break;
            }
            trace_debug(MODULE, "count of %s", envp[i]);
            uint32_t length = strlen(envp[i]) + 1;
            str_size += length;
            info->saved_envp[i] = kzmalloc(length);
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

status_t process_args_parse(process_args_t *info, process_t *proc)
{
    ENTER_FUNC("%p, %p", info, proc);
    int argc = info->argc;
    int envc = info->envc;
    size_t str_size = info->str_size;

    char **temp_argv = info->saved_argv;
    char **temp_envp = info->saved_envp;

    reg_t user_stack = (reg_t)proc->vma->start_stack;
    user_stack &= ~0xFull;
    user_stack -= str_size;
    trace_debug(MODULE, "user_stack = %p", user_stack);

    reg_t stack_base = user_stack;

    uint64_t table_size = sizeof(uint64_t) * 3;
    table_size += argc * sizeof(uint64_t);
    table_size += envc * sizeof(uint64_t);

    user_stack -= table_size;
    user_stack &= ~0xFull;

    reg_t table_cursor = user_stack;
    *((uint64_t *)table_cursor) = (uint64_t)argc;
    trace_debug(MODULE, "argc (%d) @ %p", argc, table_cursor);
    table_cursor += sizeof(uint64_t);

    trace_debug(MODULE, "argc = %u", argc);
    for (int i = 0; i < argc; i++)
    {
        size_t length = strlen(temp_argv[i]) + 1;
        memcpy((void *)stack_base, temp_argv[i], length);
        *((uint32_64_t *)table_cursor) = (uint32_64_t)stack_base;
        trace_debug(MODULE, "argv[%d] = %s @ %p", i, temp_argv[i], table_cursor);
        stack_base += length;
        table_cursor += sizeof(uint64_t);
        kfree(temp_argv[i]);
    }

    *((uint64_t *)table_cursor) = 0;
    trace_debug(MODULE, "0 @ %p", table_cursor);
    table_cursor += sizeof(uint64_t);

    if (info->envc != 0)
    {
        trace_debug(MODULE, "envc = %u", envc);
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
            trace_debug(MODULE, "envp[%d] = %s @ %p", i, temp_envp[i], table_cursor);
            stack_base += length;
            table_cursor += sizeof(uint64_t);
            kfree(temp_envp[i]);
        }
    }

    *((uint64_t *)table_cursor) = 0;
    trace_debug(MODULE, "0 @ %p", table_cursor);

    proc->vma->start_stack = (vaddr_t)user_stack;
    kfree(info);

    return argc;
}

status_t process_load(const char *path, process_t *proc)
{
    char volume[CONFIG_MAX_VOLUME_LENGTH];
    if (path_split_prefix(path, volume, NULL))
    {
        strcpy(proc->volume, volume);
    }

    vfs_node_t *fd_node;
    status_t ret = vfs_open(path, O_RDONLY, 0666, &fd_node);
    if (ret)
    {
        KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "file %s was not found", path);
    }
    ret = mmu_arch_load_table(proc->vma->page_directory);
    if (ret)
    {
        return ret;
    }
    trace_info(MODULE, "loading proc->vma = %p", proc->vma);
    ret = loader_load(fd_node, proc);

    return KERRNO_SUCCESSES;
}

status_t proc_exec(process_t *proc, const char *path, char *argv[], char *envp[])
{
    ENTER_FUNC(MODULE, "%p, %s, %p, %p", proc, path, argv, envp);

    page_table_t current_page;
    mmu_arch_current_table(&current_page);

    process_args_t *info = process_args_save(proc, argv, envp);

    if (proc->parent->state == PROC_STATE_SUSPENDED)
    {
        // mmu_arch_map_kernel(proc->page_dir);
        // page_table_t *new_table = mmu_arch_create_table();
        // mmu_arch_map_kernel(new_table);
        // mmu_arch_load_table(new_table);
        // mmu_arch_destroy_table(proc->page_dir);
        // proc->page_dir = new_table;
        // memcpy(&current_page, new_table, sizeof(page_table_t));
    }
    else
    {
        vma_free(proc->vma);
    }
    vma_allocate(&proc->vma);

    trace_info(MODULE, "loading program %s", path);
    mmu_arch_load_table(proc->vma->page_directory);
    process_load(path, proc);

    trace_info(MODULE, "loading args");
    process_args_parse(info, proc); // shit with args

    mmu_arch_load_table(&current_page); // load cr3
    // mmu_arch_flush_all();

    return KERRNO_SUCCESSES;
}

status_t proc_kernel_execve(process_t *proc, const char *path, char *argv[], char *envp[], thread_t **out_thread)
{
    ENTER_FUNC(MODULE, "%p, %s, %p, %p, %p", proc, path, argv, envp, out_thread);
    proc_exec(proc, path, argv, envp);
    trace_info(MODULE, "new thread");
    thread_t *user = thread_create_user(proc, (uint64_t)proc->vma->start_stack, 0);
    user->cpu_affinity = cpu_arch_get_current();
    user->cpu_affinity->current_proc = proc;
    user->is_main_thread = 1;

    context_arch_set_stack(&user->ctx, proc->vma->start_stack);

    context_dump(&user->ctx);

    *out_thread = user;
    return KERRNO_SUCCESSES;
}
