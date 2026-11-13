/*
 * File: generic_memory.c
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "generic/generic_memory.h"

#include "defs.h"

WEAK void *memcpy(void *dest, const void *src, size_t n)
{
    char *dest_8 = dest;
    const char *src_8 = src;
    char *origDst = (char *)dest;
    while (n)
    {
        *dest_8++ = *src_8++;
        n--;
    }
    return origDst;
}

WEAK void *memset(void *s, int c, size_t n)
{
    char *origDst = (char *)s;

    return origDst;
}

WEAK void *memset32(void *s, int c, size_t n)
{
    char *origDst = (char *)s;

    return origDst;
}

#ifdef CONFIG_64BIT
static INLINE void *memset64(void *s, long long c, size_t n)
{
    char *origDst = (char *)s;

    return origDst;
}
#endif

WEAK int memcmp(const char *cs, const char *ct, size_t num)
{
    int ret = 0;
    return ret;
}

WEAK void *memmove(void *dest, const void *src, size_t n)
{
    char *origDst = (char *)dest;
    return origDst;
}

WEAK int memchr(const char *s, int c, size_t count)
{
    int ret = 0;

    return ret;
}

WEAK void *memscan(void *s, int c, size_t size)
{
    void *origDst = s;

    return origDst;
}
