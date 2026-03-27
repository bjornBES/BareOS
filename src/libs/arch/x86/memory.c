/*
 * File: memory.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stddef.h>

#define inline_asm __asm__ __volatile__

void *memcpy(void *dest, const void *src, size_t n)
{
    char *origDst = dest;
    inline_asm(
        "rep movsb" : "+S"(src),
        "+D"(dest), "+c"(n) : : "eax", "memory");
    return origDst;
}

void *memset(void *s, int c, size_t n)
{
    char *origDst = s;
    inline_asm(
        "rep stosb"
        : "+D"(s), "+c"(n)
        : "a" (c)
        : "memory");

    return origDst;
}

int memcmp(const char *cs, const char *ct, size_t num)
{
    int ret;
    inline_asm(
        "xor %[result], %[result]\n\t"
        "repe cmpsb\n\t"
        "setnz al"
        : "+D"(cs), "+S"(ct), "+c" (num), [result] "=a" (ret)
        :
        : "memory");

    return ret;
}

void *memmove(void *dest, const void *src, size_t n)
{
    char *origDst = dest;
    inline_asm(
        "cmp %[dst], %[src]\n\t"
        "je done_%=\n\t"
        "jc forward_%=\n\t"
        "lea %[src], [%[src] + %[cnt] - 1]\n\t"
        "lea %[dst], [%[dst] + %[cnt] - 1]\n\t"
        "std\n\t"
        "rep movsb\n\t"
        "cld\n\t"
        "jmp done_%=\n"
    "forward_%=:\n\t"
        "cld\n\t"
        "rep movsb\n"
    "done_%=:"
        : [src] "+S"(src), [dst] "+D"(dest), [cnt] "+c"(n)
        :
        : "eax", "memory");
    return origDst;
}

int memchr(const char *s, int c, size_t count)
{
    int ret;
    inline_asm(
        "rep scasb\n\t"
        "jne not_found_%=\n\t"
        "lea %[result], [%[src] - 1]\n\t"
        "jmp done_%=\n"
    "not_found_%=:\n\t"
        "xor %[result], %[result]\n"
    "done_%=:"
        : [src] "+S"(s), [result] "=a"(ret), "+c"(count)
        :
        : "memory");
    return ret;
}

void *memscan(void*s, int c, size_t size)
{
    void *origDst = s;
    inline_asm(
        "repne scasb"
        : "+S"(s), "+c"(size)
        : "a"(c)
        : "memory");
    return origDst;
}