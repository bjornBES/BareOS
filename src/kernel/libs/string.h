/*
 * File: string.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <core/Defs.h>
#include <stdint.h>
#include <stddef.h>

#include <base_string.h>

#define UNICODE_COMBINING_ACUTE 0x0301
#define UNICODE_COMBINING_CARON 0x030C

#define BOOT_TO_STRING ? "TRUE" : "FALSE" 

typedef char* string;

int strcasecmp(const char *a, const char *b);
int strncasecmp(const char *a, const char *b, size_t count);

size_t strspn(const char* s1, const char* s2);
size_t strcspn(const char *s1, const char *s2);
char *strrchr(const char *s, int c);

void itoa(char *buf, uint32_t n, int base);
void atoi(char* str, int* a);
size_t atou(const char* str);
const char* atou_return(const char *str, size_t* result);

uint32_t strcrl(string str, const char what, const char with);
uint32_t str_begins_with(string str, string with);
uint32_t str_backspace(string str, char c);
uint32_t strcount(string str, char c);
char* strtok(string str, const char* delim);

char* strdup(const char* str1);

uint16_t* utf16_to_codepoint(uint16_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);
