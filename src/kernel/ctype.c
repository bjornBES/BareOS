/*
 * File: ctype.c
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ctype.h"

bool islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}

bool isupper(char chr)
{
    return chr >= 'A' && chr <= 'Z';
}

char toupper(char chr)
{
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}

char tolower(char chr)
{
    return isupper(chr) ? (chr - 'A' + 'a') : chr;
}

// internal test if char is a digit (0-9)
// @return true if char is a digit
bool isdigit(char chr)
{
    return (chr >= '0') && (chr <= '9');
}

bool isalnum(char chr)
{
    return isalpha(chr) || isdigit(chr);
}

bool isalpha(char chr)
{
    return islower(chr) || isupper(chr);
}

bool iscntrl(char chr)
{
    return (chr >= 0) && (chr <= 0x1F);
}

bool isgraph(char chr)
{
    return chr >= 0x21 && chr <= 0x7E;
}

bool isprint(char chr)
{
    return chr >= 0x21 && chr <= 0x7E;
}

bool ispunct(char chr)
{
    return (chr >= 0x21 && chr <= 0x2F) || /*  ! to /  */
        (chr >= 0x3A && chr <= 0x40) ||    /*  : to @  */
        (chr >= 0x7A && chr <= 0x7E);      /*  { to ~  */
}

bool isspace(char chr)
{
    return chr == ' ' || ('\t' <= chr && chr <= '\r');
}

bool isxdigit(char chr)
{
    return (chr >= '0' && chr <= '9') ||
        (chr >= 'a' && chr <= 'f') ||
        (chr >= 'A' && chr <= 'F');
}
