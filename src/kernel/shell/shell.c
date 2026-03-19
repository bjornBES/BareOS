/*
 * File: shell.c
 * File Created: 13 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "shell.h"
#include "drivers/IO/Keyboard/Keyboard.h"
#include "VFS/vfs.h"
#include "malloc.h"
#include "debug/debug.h"
#include "task/process.h"

#include <stdio.h>
#include <core/video/VGATextDevice.h>
#include <memory.h>
#include <string.h>

#include <stdint.h>
#include <stdbool.h>

#define MODULE "SHELL"
#define MAX_ARGS 16
#define INPUT_BUFFER_SIZE 256
#define COMMAND_BUFFER_SIZE 64

void shell_read_line(char *input_buffer)
{
    uint16_t buffer_index = 0;
    int x = 0, y = 0;
    vga_get_cursor(&x, &y);

    while (true)
    {
        key_event ke;
        if (keyboard_get_key(&ke) == false)
        {
            continue;
        }

        uint32_t c = keyboard_get_ascii(&ke);
        if (c == 0)
        {
            continue;
        }
        switch (c)
        {
        case '\b':
            if (buffer_index > 0)
            {
                x--;
                buffer_index--;
                input_buffer[buffer_index] = '\0';
                vga_set_cursor(x, y);
                printf(" ");
                vga_set_cursor(x, y);
            }
            break;
        case '\r':
            printf("\n");
            return;

        default:
            input_buffer[buffer_index] = c;
            buffer_index++;
            printf("%c", c);
            x++;
            break;
        }
        vga_set_cursor(x, y);
    }
}

void shell_parse_arguments(char *input, int *argc, char **argv)
{
    *argc = 0;
    char *token = strtok(input, " ");
    while (token && *argc < MAX_ARGS)
    {
        argv[(*argc)++] = token;
        token = strtok(NULL, " ");
    }
}

void shell_enter()
{
    char *input_buffer = (char *)malloc(INPUT_BUFFER_SIZE);
    char *mount_path = (char *)malloc(64);

    vga_clear();
    printf("BES OS v0.1\n");
    printf("This operating system is under construction.\n\n");

    while (true)
    {
        printf("%s: ", mount_path);
        memset(input_buffer, 0, INPUT_BUFFER_SIZE);
        shell_read_line(input_buffer);

        int argc = 0;
        char *argv[MAX_ARGS] = {0};
        shell_parse_arguments(input_buffer, &argc, argv);
        if (argc == 0)
            continue;

        char *command = argv[0];
        log_debug(MODULE, "command = %s", command);

        if (!strcmp(command, "EXEC"))
        {
            if (argc < 2)
            {
                printf("ERROR: ");
                printf("USAGE: EXEC [FILENAME]\n");
                continue;
            }

            char path[320];
            int count = sprintf(path, "%sbin/%s", mount_path, argv[1]);
            path[count] = '\0';
            exec(path, argv);
        }
        else if (!strcmp(command, "LS"))
        {
            if (argc < 2)
            {
                printf("ERROR: ");
                printf("USAGE ");
                continue;
            }
            char path[320];
            int count = sprintf(path, "%s%s", mount_path, argv[1]);
            path[count] = '\0';
            fd_t dir = VFS_open(path);
            vfs_dirent entry;
            printf("at %s", path);
            while (VFS_read_dir(dir, &entry) == 1)
            {
                printf("\t%u\n", entry.name);
            }
        }
        else if (!strcmp(command, "RUN"))
        {
            int count = 0;
            mount_point **points = vfs_get_mount_points(&count);
            int mount_index = 2;
            mount_point *mnt = points[mount_index - 1];
            printf("%u: MOUNT POINT %s IS %s\n", mount_index, mnt->path, mnt->dev->name);
            strcpy(mount_path, mnt->path);
            char path[320];
            count = sprintf(path, "%sbin/INIT.ELF", mount_path);
            path[count] = '\0';
            exec(path, argv);
        }
        else if (!strcmp(command, "MOUNT"))
        {
            // MOUNT [list/]
            if (argc < 2)
            {
                printf("ERROR: ");
                printf("USAGE ");
                continue;
            }

            char *sub_command = argv[1];
            int count = 0;
            mount_point **points = vfs_get_mount_points(&count);
            if (!strcmp(sub_command, "LIST"))
            {
                for (size_t i = 0; i < count; i++)
                {
                    mount_point *mount = points[i];
                    printf("%u: MOUNT POINT %s IS %s\n", i + 1, mount->path, mount->dev->name);
                }
            }
            else
            {
                int mount_index = -1;
                atoi(argv[1], &mount_index);
                if (mount_index == 0)
                {
                    printf("ERROR: ");
                    continue;
                }
                mount_point *mnt = points[mount_index - 1];
                printf("%u: MOUNT POINT %s IS %s\n", mount_index, mnt->path, mnt->dev->name);
                strcpy(mount_path, mnt->path);
            }
        }
    }
}
