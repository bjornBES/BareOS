/*
 * File: arch_string.h
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "defs.h"


INLINE char *strcpy(char *dest, const char *src)
{
    char *origDst = dest;

    inline_asm("L1_%=:"
               "lodsb\n\t"
               "stosb\n\t"
               "test al, al\n\t"
               "jne L1_%=" : "+S"(src),
               "+D"(dest) : : "eax", "memory");

    return origDst;
}

INLINE char *strncpy(char *dest, const char *src, size_t count)
{
    char *origDst = dest;
    inline_asm(
        "L1_%=:\n\t"
        "dec %[cnt]\n\t"
        "js L2_%=\n\t"
        "lodsb\n\t"
        "stosb\n\t"
        "test al, al\n\t"
        "jne L1_%=\n\t"
        "rep stosb\n\t"
        "L2_%=:" : "+S"(src),
        "+D"(dest), [cnt] "+c"(count) : : "eax", "memory");

    return origDst;
}

INLINE char *strcat(char *dest, const char *src)
{
    char *origDst = dest;
    inline_asm(
        "repne scasb\n\t"
        "dec %[dst]\n\t"
        "L1_%=:\n\t"
        "lodsb\n\t"
        "stosb\n\t"
        "test al, al\n\t"
        "jne L1_%=" : "+S"(src),
        [dst] "+D"(dest) : : "eax", "memory");

    return origDst;
}

INLINE char *strncat(char *dest, const char *src, size_t count)
{
    char *origDst = dest;
    inline_asm(
        "L1_%=:\n\t"
        "lodsb\n\t"
        "test al, al\n\t"
        "jnz L1_%=\n\t"
        "dec %[dst]\n\t"
        "L2_%=:\n\t"
        "dec %[cnt]\n\t"
        "js L3_%=\n\t"
        "lodsb\n\t"
        "stosb\n\t"
        "test al, al\n\t"
        "jne L2_%=\n\t"
        "jmp L4_%=\n\t"
        "L3_%=:\n\t"
        "xor eax, eax\n\t"
        "stosb\n\t"
        "L4_%=:" : "+S"(src),
        [dst] "+D"(dest), [cnt] "+c"(count) : : "eax", "memory");

    return origDst;
}

INLINE int strcmp(const char *cs, const char *ct)
{
    int res;
    inline_asm(
        "L1_%=:\n\t"
        "lodsb\n\t"
        "scasb\n\t"
        "jne L2_%=\n\t"
        "test al, al\n\t"
        "jne L1_%=\n\t"
        "xor %[result], %[result]\n\t"
        "jmp L3_%=\n"
        "L2_%=:\n\t"
        "sbb %[result], %[result]\n\t"
        "or %[result], 1\n"
        "L3_%=:" : [result] "=a"(res),
        "+S"(cs), "+D"(ct) : : "memory");
    return res;
}

INLINE int strncmp(const char *cs, const char *ct, size_t count)
{
    int res;
    inline_asm(
        "L1_%=:\n\t"
        "dec %[cnt]\n\t"
        "js L2_%=\n\t"
        "lodsb\n\t"
        "scasb\n\t"
        "jne L3_%=\n\t"
        "test al, al\n\t"
        "jne L1_%=\n"
        "L2_%=:\n\t"
        "xor %[result], %[result]\n\t"
        "jmp L4_%=\n"
        "L3_%=:\n\t"
        "sbb %[result], %[result]\n\t"
        "or %[result], 1\n"
        "L4_%=:" : [result] "=a"(res),
        "+S"(cs), "+D"(ct), [cnt] "+c"(count) : : "memory");
    return res;
}

INLINE char *strchr(const char *s, int c)
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

INLINE size_t strlen(const char *s)
{
    size_t res = __SIZE_MAX__;
    inline_asm(
        "repne scasb" : [result] "+c"(res), "+D"(s) : "a"(0) : "memory");
    return ~res - 1;
}

INLINE size_t strnlen(const char *s, size_t count)
{
    size_t res;
    inline_asm(
        "mov %[result], %[src]\n\t" // res = s
        "jmp L2_%=\n"
        "L1_%=:\n\t"
        "cmp byte ptr [%[result]], 0\n\t"
        "je L3_%=\n\t"
        "inc %[result]\n"
        "L2_%=:\n\t"
        "dec %[cnt]\n\t"
        "cmp %[cnt], -1\n\t"
        "jne L1_%=\n"
        "L3_%=:\n\t"
        "sub %[result], %[src]" : [result] "=a"(res),
        [cnt] "+d"(count) : [src] "c"(s) : "memory");
    return res;
}

