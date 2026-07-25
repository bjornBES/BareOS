/*
 * File: string.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "base_string.h"
#include "base_memory.h"
#include "base_ctype.h"

#include <stdint.h>
#include <stddef.h>

char *strcpy(char *dest, const char *src)
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

int strcmp(const char *cs, const char *ct)
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

char *strchr(const char *s, int c)
{
	while (*s != (char)c)
		if (*s++ == '\0')
			return NULL;
	return (char *)s;
}

size_t strlen(const char *s)
{
    size_t res = __SIZE_MAX__;
    inline_asm(
        "repne scasb" : [result] "+c"(res), "+D"(s) : "a"(0) : "memory");
    return ~res - 1;
}

wchar_t *utf16_to_codepoint(wchar_t *string, int *codepoint)
{
    int c1 = *string;
    ++string;

    if (c1 >= 0xd800 && c1 < 0xdc00)
    {
        int c2 = *string;
        ++string;
        *codepoint = ((c1 & 0x3ff) << 10) + (c2 & 0x3ff) + 0x10000;
    }
    *codepoint = c1;
    return string;
}

/* Encoding
   The following byte sequences are used to represent a
   character.  The sequence to be used depends on the UCS code
   number of the character:

   0x00000000 - 0x0000007F:
       0xxxxxxx

   0x00000080 - 0x000007FF:
       110xxxxx 10xxxxxx

   0x00000800 - 0x0000FFFF:
       1110xxxx 10xxxxxx 10xxxxxx

   0x00010000 - 0x001FFFFF:
       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

   [... removed obsolete five and six byte forms ...]

   The xxx bit positions are filled with the bits of the
   character code number in binary representation.  Only the
   shortest possible multibyte sequence which can represent the
   code number of the character can be used.

   The UCS code values 0xd800–0xdfff (UTF-16 surrogates) as well
   as 0xfffe and 0xffff (UCS noncharacters) should not appear in
   conforming UTF-8 streams.
*/

char *codepoint_to_utf8(int codepoint, char *stringOutput)
{
    if (codepoint <= 0x7F)
    {
        *stringOutput = (char)codepoint;
        stringOutput++;
    }
    else if (codepoint <= 0x7FF)
    {
        *stringOutput = 0xC0 | ((codepoint >> 6) & 0x1F);
        stringOutput++;
        *stringOutput = 0x80 | (codepoint & 0x3F);
        stringOutput++;
    }
    else if (codepoint <= 0xFFFF)
    {
        *stringOutput = 0xE0 | ((codepoint >> 12) & 0xF);
        stringOutput++;
        *stringOutput = 0x80 | ((codepoint >> 6) & 0x3F);
        stringOutput++;
        *stringOutput = 0x80 | (codepoint & 0x3F);
        stringOutput++;
    }
    else if (codepoint <= 0x1FFFFF)
    {
        *stringOutput = 0xF0 | ((codepoint >> 18) & 0x7);
        stringOutput++;
        *stringOutput = 0x80 | ((codepoint >> 12) & 0x3F);
        stringOutput++;
        *stringOutput = 0x80 | ((codepoint >> 6) & 0x3F);
        stringOutput++;
        *stringOutput = 0x80 | (codepoint & 0x3F);
        stringOutput++;
    }
    return stringOutput;
}
