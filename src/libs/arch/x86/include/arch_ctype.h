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

static INLINE bool islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}

static INLINE bool isupper(char chr)
{
    return chr >= 'A' && chr <= 'Z';
}

static INLINE char toupper(char chr)
{
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}

static INLINE char tolower(char chr)
{
    return isupper(chr) ? (chr - 'A' + 'a') : chr;
}

// internal test if char is a digit (0-9)
// @return true if char is a digit
static INLINE bool isdigit(char chr)
{
    return (chr >= '0') && (chr <= '9');
}

static INLINE bool isalpha(char chr)
{
    return islower(chr) || isupper(chr);
}

static INLINE bool isalnum(char chr)
{
    return isalpha(chr) || isdigit(chr);
}

static INLINE bool iscntrl(char chr)
{
    return (chr >= 0) && (chr <= 0x1F);
}

static INLINE bool isgraph(char chr)
{
    return chr >= 0x21 && chr <= 0x7E;
}

static INLINE bool isprint(char chr)
{
    return chr >= 0x21 && chr <= 0x7E;
}

static INLINE bool ispunct(char chr)
{
    return (chr >= 0x21 && chr <= 0x2F) || /*  ! to /  */
        (chr >= 0x3A && chr <= 0x40) ||    /*  : to @  */
        (chr >= 0x7A && chr <= 0x7E);      /*  { to ~  */
}

static INLINE bool isspace(char chr)
{
    return chr == ' ' || ('\t' <= chr && chr <= '\r');
}

static INLINE bool isxdigit(char chr)
{
    return (chr >= '0' && chr <= '9') ||
        (chr >= 'a' && chr <= 'f') ||
        (chr >= 'A' && chr <= 'F');
}