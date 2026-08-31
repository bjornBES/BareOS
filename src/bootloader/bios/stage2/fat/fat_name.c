/*
 * File: fat_name.c
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdint.h>
#include "ctype.h"
#include "memory.h"
#include "string.h"

int fat_83_to_name(const char raw83[11], char out[13])
{
    if (!raw83 || !out)
    {
        return 1;
    }

    // Deleted entry
    if (raw83[0] == (char)0xE5)
    {
        return 1;
    }
    // Empty (end-of-directory) entry
    if (raw83[0] == (char)0x00)
    {
        return 1;
    }

    char *p = out;

    // ---- Name part (bytes 0-7) ----
    for (int i = 0; i < 8; i++)
    {
        uint8_t c = raw83[i];
        // trailing spaces = end of name
        if (c == ' ')
        {
            break;
        }
        // Kanji / 0xE5 escape
        if (i == 0 && c == 0x05)
        {
            c = 0xE5;
        }
        *p++ = (char)c;
    }

    // ---- Extension part (bytes 8-10) ----
    // Check if there is a non-space extension
    int has_ext = 0;
    for (int i = 8; i < 11; i++)
    {
        if (raw83[i] != ' ')
        {
            has_ext = 1;
            break;
        }
    }

    if (has_ext)
    {
        *p++ = '.';
        for (int i = 8; i < 11; i++)
        {
            if (raw83[i] == ' ')
            {
                break;
            }
            *p++ = (char)raw83[i];
        }
    }

    *p = '\0';
    return 0;
}

int fat_name_to_83(const char *name, char out83[12])
{
    if (!name || !out83 || name[0] == '\0')
    {
        return 1;
    }

    // Pre-fill with spaces
    out83 = memset(out83, ' ', 12);
    out83[11] = 0;

    // Find the LAST dot to split name / extension.
    // Special case: a leading dot (e.g. ".hidden") is part of the name, not
    // a name/ext separator.
    const char *dot = (const char *)0;
    for (const char *p = name + 1; *p; p++)
    { // start at +1 to skip leading dot
        if (*p == '.')
        {
            dot = p;
        }
    }

    const char *ext_start = dot ? dot + 1 : (const char *)0;
    int name_len = dot ? (int)(dot - name) : (int)strlen(name);

    // ---- Write name (bytes 0-7) ----
    int ni = 0;
    for (int i = 0; i < name_len && ni < 8; i++)
    {
        char c = toupper(name[i]);
        // skip spaces (illegal in FAT)
        if (c == ' ' || c == '\0')
        {
            continue;
        }
        out83[ni++] = (char)c;
    }

    // Kanji / deleted-entry escape
    if (out83[0] == 0xE5)
    {
        out83[0] = 0x05;
    }

    // ---- Write extension (bytes 8-10) ----
    if (ext_start)
    {
        int ei = 8;
        for (int i = 0; ext_start[i] != '\0' && ei < 11; i++)
        {
            char c = toupper(ext_start[i]);
            if (c == ' ')
            {
                continue;
            }
            out83[ei++] = (char)c;
        }
    }

    return 0;
}

