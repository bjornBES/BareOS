/*
 * File: string.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <defs.h>
#include <stdint.h>
#include <stddef.h>

#define UNICODE_COMBINING_ACUTE 0x0301
#define UNICODE_COMBINING_CARON 0x030C

#define BOOL_TO_STRING ? "TRUE" : "FALSE" 

typedef char* string;

char* strcpy(char* dst, const char* src);
int strcmp(const char* a, const char* b);
char* strchr(const char* str, int chr);
uint32_t strlen(const char* str);

wchar_t* utf16_to_codepoint(wchar_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);
