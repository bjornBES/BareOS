/*
 * File: debug.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include "debug.h"
#include "stdio.h"
#include "core/printfDriver/printf.h"

static const char* const g_LogSeverityColors[] =
{
    [LVL_DEBUG]        = "\033[2;37m",
    [LVL_INFO]         = "\033[37m",
    [LVL_WARN]         = "\033[1;33m",
    [LVL_ERROR]        = "\033[1;31m",
    [LVL_CRITICAL]     = "\033[1;37;41m",
};

static const char* const g_ColorReset = "\033[0m";

void logf(const char* module, DebugLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    if (level < MIN_LOG_LEVEL)
    return;
    
    /*
    char* severity_color = g_LogSeverityColors[level];

    do
    {
        if (level == LVL_DEBUG)
        {
            severity_color = "\033[2;37m\0";
            break;
        }
        if (level == LVL_INFO)
        {
            severity_color = "\033[37m\0";
            break;
        }
        if (level == LVL_WARN)
        {
            severity_color = "\033[1;33m\0";
            break;
        }
        if (level == LVL_ERROR)
        {
            severity_color = "\033[1;31m\0";
            break;
        }
        if (level == LVL_CRITICAL)
        {
            severity_color = "\033[1;37;41m\0";
            break;
        }
    } while (0); */

    // 0x6f 0x6f 0x62 0x20    

    fprintf(VFS_FD_DEBUG, "%s", g_LogSeverityColors[level]); // set color depending on level
    fprintf(VFS_FD_DEBUG, "[%s] ", module);             // write module
    vprintf(VFS_FD_DEBUG, fmt, args);                   // write text
    fputs(g_ColorReset, VFS_FD_DEBUG);                  // reset format
    fputs("\n", VFS_FD_DEBUG);                          // newline

    va_end(args);  
}

void strlogf(DebugLevel level, const char* str)
{
    if (level < MIN_LOG_LEVEL)
    return;
    
    fputs(g_LogSeverityColors[level], VFS_FD_DEBUG);    // set color depending on level
    fprintf(VFS_FD_DEBUG, str);                         // write text
    fputs(g_ColorReset, VFS_FD_DEBUG);                  // reset format
    fputs("\n", VFS_FD_DEBUG);                          // newline
}

void write_error(DebugLevel level, const char* module, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vprintf(VFS_FD_STDERR, fmt, args);

    fputs(g_LogSeverityColors[level], VFS_FD_DEBUG);    // set color depending on level
    fprintf(VFS_FD_DEBUG, "[%s] ", module);             // write module
    vprintf(VFS_FD_DEBUG, fmt, args);                   // write text
    fputs(g_ColorReset, VFS_FD_DEBUG);                  // reset format
    fputs("\n", VFS_FD_DEBUG);                          // newline

    va_end(args);
}
