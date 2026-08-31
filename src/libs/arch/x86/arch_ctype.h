/*
 * File: ctype.h
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */


#pragma once

#include "types.h"
#include "defs.h"

INLINE bool islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}

INLINE bool isupper(char chr)
{
    return chr >= 'A' && chr <= 'Z';
}

INLINE char toupper(char chr)
{
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}

INLINE char tolower(char chr)
{
    return isupper(chr) ? (chr - 'A' + 'a') : chr;
}

// internal test if char is a digit (0-9)
// @return true if char is a digit
INLINE bool isdigit(char chr)
{
    return (chr >= '0') && (chr <= '9');
}

INLINE bool isalpha(char chr)
{
    return islower(chr) || isupper(chr);
}

INLINE bool isalnum(char chr)
{
    return isalpha(chr) || isdigit(chr);
}

INLINE bool iscntrl(char chr)
{
    return (chr >= 0) && (chr <= 0x1F);
}

INLINE bool isgraph(char chr)
{
    return chr >= 0x21 && chr <= 0x7E;
}

INLINE bool isprint(char chr)
{
    return chr >= 0x21 && chr <= 0x7E;
}

INLINE bool ispunct(char chr)
{
    return (chr >= 0x21 && chr <= 0x2F) || /*  ! to /  */
        (chr >= 0x3A && chr <= 0x40) ||    /*  : to @  */
        (chr >= 0x7A && chr <= 0x7E);      /*  { to ~  */
}

INLINE bool isspace(char chr)
{
    return chr == ' ' || ('\t' <= chr && chr <= '\r');
}

INLINE bool isxdigit(char chr)
{
    return (chr >= '0' && chr <= '9') ||
        (chr >= 'a' && chr <= 'f') ||
        (chr >= 'A' && chr <= 'F');
}