/*
 * File: memory.h
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

inline static void *memcpy(void *dest, const void *src, size_t n)
{
    char *origDst = (char*)dest;
    __asm__(
        "rep movsb" : "+S"(src),
        "+D"(dest), "+c"(n) : : "eax", "memory");
    return origDst;
}

inline static void *memset(void *s, int c, size_t n)
{
    char *origDst = (char*)s;
    __asm__(
        "rep stosb" : "+D"(s), "+c"(n) : "a"(c) : "memory");

    return origDst;
}
inline static void *memset32(void *s, int c, size_t n)
{
    char *origDst = (char*)s;
    __asm__(
        "rep stosd" : "+D"(s), "+c"(n) : "a"(c) : "memory");

    return origDst;
}

inline static int memcmp(const char *cs, const char *ct, size_t num)
{
    int ret;
    __asm__(
        "xor %[result], %[result]\n\t"
        "repe cmpsb\n\t"
        "setnz al" : "+D"(cs),
        "+S"(ct), "+c"(num), [result] "=a"(ret) : : "memory");

    return ret;
}

inline static void *memmove(void *dest, const void *src, size_t n)
{
    char *origDst = (char*)dest;
    __asm__(
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
        "done_%=:" : [src] "+S"(src),
        [dst] "+D"(dest), [cnt] "+c"(n) : : "eax", "memory");
    return origDst;
}

inline static int memchr(const char *s, int c, size_t count)
{
    int ret;
    __asm__(
        "rep scasb\n\t"
        "jne not_found_%=\n\t"
        "lea %[result], [%[src] - 1]\n\t"
        "jmp done_%=\n"
        "not_found_%=:\n\t"
        "xor %[result], %[result]\n"
        "done_%=:" : [src] "+S"(s), [value] "+d"(c),
        [result] "=a"(ret), "+c"(count) : : "memory");

    return ret;
}

inline static void *memscan(void *s, int c, size_t size)
{
    void *origDst = s;
    __asm__(
        "repne scasb" : "+S"(s), "+c"(size) : "a"(c) : "memory");
    return origDst;
}