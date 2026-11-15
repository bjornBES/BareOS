/*
 * File: cmdline_init.c
 * File Created: 15 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "cmdline/cmdline_init.h"

#include "debug/debug.h"

#include "string.h"

extern const cmdline_param_t __cmdline_params_start[];
extern const cmdline_param_t __cmdline_params_end[];

status_t cmdline_call_early(char *key, const char *value)
{
    {
        cmdline_param_t *funcs = (cmdline_param_t *)__cmdline_params_start;
        while (funcs != NULL && funcs->handler != NULL)
        {
            if (funcs->early)
            {
                if (strcmp(funcs->key, key) == 0)
                {
                    trace_debug("cmdline-init", "running function %p", *funcs);
                    status_t ret = (*funcs->handler)(value);
                    if (ret != KERRNO_SUCCESSES)
                    {
                        KERNEL_PANIC("cmdline-init", "init function at %p returned non zero", *funcs);
                    }
                }
                funcs++;
            }
        }
    }
    return KERRNO_SUCCESSES;
}

status_t cmdline_call_later(char *key, const char *value)
{
    {
        cmdline_param_t *funcs = (cmdline_param_t *)__cmdline_params_start;
        while (funcs != NULL && funcs->handler != NULL)
        {
            if (funcs->early == 0)
            {
                if (strcmp(funcs->key, key) == 0)
                {
                    trace_debug("cmdline-init", "running function %p", *funcs);
                    status_t ret = (*funcs->handler)(value);
                    if (ret != KERRNO_SUCCESSES)
                    {
                        KERNEL_PANIC("cmdline-init", "init function at %p returned non zero", *funcs);
                    }
                }
            }
            funcs++;
        }
    }
    return KERRNO_SUCCESSES;
}
