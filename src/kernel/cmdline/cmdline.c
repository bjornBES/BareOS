/*
 * File: cmdline.c
 * File Created: 15 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "cmdline/cmdline.h"
#include "cmdline/cmdline_init.h"

#include "lists/hash.h"

#include "init.h"
#include "ctype.h"
#include "memory.h"
#include "string.h"

#define OPTION(ops) if (strcmp(cmdline, CONCAT(ops, "=")) == 0)

#define CMDLINE_MAX_ENTRIES 64

#define MODULE      "cmdline"

static hash_map_t *cmdline_map;

static char saved_cmdline[4096];

extern status_t cmdline_call_early(char *key, const char *value);
extern status_t cmdline_call_later(char *key, const char *value);

status_t cmdline_early_parse()
{
    const char *cmdline = saved_cmdline;
    trace_debug(MODULE, "kernel command line = \"%s\"", cmdline);
    bool last_entry = false;
    while (*cmdline)
    {
        while (*cmdline == ' ')
        {
            cmdline++;
        }

        // trace_debug(MODULE, "cmdline = %s", cmdline);
        char *end = strchrnul(cmdline, ' ');

        if (end)
        {
            *end = '\0';
        }
        else
        {
            last_entry = true;
            end = (char *)cmdline;
            while (*end)
            {
                end++;
            }
            // trace_debug(MODULE, "cmdline = \"%s\"", cmdline);
        }

        const char *value = NULL;
        char *expr = strchr(cmdline, '=');
        // trace_debug(MODULE, "expr = %p \"%s\"", expr, expr);
        if (expr)
        {
            *expr = '\0';
            value = (const char *)expr + 1;
            // trace_debug(MODULE, "expr = \"%s\"", expr + 1);
        }

        char *key = (char *)cmdline;

        trace_debug(MODULE, "key: \"%s\" = value: \"%s\"", key, value);
        cmdline_call_early(key, value);
        if (!last_entry)
        {
            *end = ' ';
            cmdline = end + 1;
            *expr = '=';
        }
        else
        {
            cmdline = end;
        }
    }

    return KERRNO_SUCCESSES;
}

status_t cmdline_early_init(boot_params_t *boot_params)
{
    hexdump((char *)boot_params->cmdline_addr, boot_params->cmdline_size, 16);
    memcpy(saved_cmdline, (void *)boot_params->cmdline_addr, boot_params->cmdline_size);

    return cmdline_early_parse();
}

status_t cmdline_parse()
{
    const char *cmdline = saved_cmdline;
    trace_debug(MODULE, "kernel command line = \"%s\"", cmdline);
    bool last_entry = false;
    while (*cmdline)
    {
        while (*cmdline == ' ')
        {
            cmdline++;
        }

        // trace_debug(MODULE, "cmdline = %s", cmdline);
        char *end = strchrnul(cmdline, ' ');

        if (end)
        {
            *end = '\0';
        }
        else
        {
            last_entry = true;
            end = (char *)cmdline;
            while (*end)
            {
                end++;
            }
            // trace_debug(MODULE, "cmdline = \"%s\"", cmdline);
        }

        const char *value = NULL;
        char *expr = strchr(cmdline, '=');
        // trace_debug(MODULE, "expr = %p \"%s\"", expr, expr);
        if (expr)
        {
            *expr = '\0';
            value = (const char *)expr + 1;
            // trace_debug(MODULE, "expr = \"%s\"", expr + 1);
        }

        char *key = (char *)cmdline;

        trace_debug(MODULE, "key: \"%s\" = value: \"%s\"", key, value);

        cmdline_entry_t *entry = kzmalloc(sizeof(cmdline_entry_t));
        entry->key = key;
        entry->val = value;
        hash_insert(cmdline_map, key, entry);

        cmdline_call_later(key, value);

        if (!last_entry)
        {
            *end = ' ';
            cmdline = end + 1;
            *expr = '=';
        }
        else
        {
            cmdline = end;
        }
    }

    return KERRNO_SUCCESSES;
}

status_t cmdline_init()
{
    hash_map_create(&cmdline_map, CMDLINE_MAX_ENTRIES);
    return cmdline_parse();
}
CORE_INITCALL(cmdline_init);
