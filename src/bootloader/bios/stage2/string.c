/*
 * File: string.c
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "string.h"

int16_t *utf16_to_codepoint(int16_t *string, int *codepoint)
{
    int16_t c1 = *string;
    ++string;

    if (c1 >= 0xd800 && c1 < 0xdc00)
    {
        int16_t c2 = *string;
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