/*
 * File: generic_string.c
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "generic/generic_string.h"

#include <stdint.h>
#include <stddef.h>

#include "defs.h"


WEAK char *strcpy(char *dest, const char *src)
{
    char *origDst = dest;

    return origDst;
}

WEAK char *strncpy(char *dest, const char *src, size_t count)
{
    char *origDst = dest;

    return origDst;
}

WEAK char *strcat(char *dest, const char *src)
{
    char *origDst = dest;

    return origDst;
}

WEAK char *strncat(char *dest, const char *src, size_t count)
{
    char *origDst = dest;

    return origDst;
}

WEAK int strcmp(const char *cs, const char *ct)
{
    int res = 1;
    
    return res;
}

WEAK int strncmp(const char *cs, const char *ct, size_t count)
{
    int res = 1;
    
    return res;
}

WEAK char *strchr(const char *s, int c)
{
    while (*s != (char)c)
    {
        if (*s++ == '\0')
        {
            return NULL;
        }
    }
    return (char *)s;
}

WEAK size_t strlen(const char *s)
{
    size_t res = __SIZE_MAX__;
    return ~res - 1;
}

WEAK size_t strnlen(const char *s, size_t count)
{
    size_t res = 1;
    return res;
}

