/*
 * File: cmdline.c
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "cmdline/cmdline.h"

#include "memory.h"
#include "memdefs.h"
#include "stdio.h"

int cmdline_early_parse(partition_t *part, fat_file_t *file, char **out_kernel_path, boot_params_t *boot_params)
{
    bool found_kernel = false;
    char *kernel_cmdline = (char *)MEMORY_CMDLINE_ADDR;
    size_t kernel_cmdline_size = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    boot_params->cmdline_addr = (uint64_t)kernel_cmdline;
#pragma GCC diagnostic pop
    char cmdline_stack[512];
    char *cmdline = cmdline_stack;
    fat_read(part, file, sizeof(cmdline_stack), cmdline_stack);
    printf("bp command line = \"%s\"\n", cmdline_stack);
    while (*cmdline)
    {
        if (memcmp(cmdline, "load_kernel=", 12) == 0)
        {
            if (found_kernel)
            {
                return 0;
            }
            cmdline += 12;
            char path[64];
            memset(path, 0, sizeof(path));
            int i = 0;
            while (*cmdline != '\n' && *cmdline != '\r' && *cmdline)
            {
                path[i] = *cmdline;
                cmdline++;
                i++;
            }
            if (fat_exist(part, path))
            {
                memcpy(*out_kernel_path, path, i);
                found_kernel = true;
            }
        }
        while (*cmdline == '\n' || *cmdline == '\r')
        {
            cmdline++;
        }
        while (memcmp(cmdline, "load_kernel=", 12) && *cmdline)
        {
            *kernel_cmdline = *cmdline;
            kernel_cmdline_size++;
            kernel_cmdline++;
            cmdline++;
        }
    }

    boot_params->cmdline_size = kernel_cmdline_size;

    if (!*cmdline)
    {
        return 0;
    }

    return 1;
}
