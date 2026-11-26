/*
 * File: string_lib.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "uapi_string.h"

#include "types.h"

#define UNICODE_COMBINING_ACUTE 0x0301
#define UNICODE_COMBINING_CARON 0x030C

#define BOOL_TO_STRING          ? "TRUE" : "FALSE"

typedef char *string;

size_t strlen(const char* str);

// char* strcpy(char* dst, char* src);
// char* strncpy(char* dst, const char* src, size_t num);

#define strscpy(_dst, _src, _out_size)           \
    kstrscpy(_dst, _src, sizeof(_dst), _out_size)

// char* strcat(char* dest, const char* src);
// char* strncat(char* dest, const char* src, size_t num);

// int strcmp(const char* a, const char* b);
// int strncmp(const char* a, const char* b, size_t num);

// const char* strchr(const char* str, char chr);
char *strrchr(const char *s, int c);

// strstr

char *strtok(string str, const char *delim);
// strtok_r

// strdup
// strndup

// strerror

size_t strspn(const char *s1, const char *s2);
size_t strcspn(const char *s1, const char *s2);

// strpbrk

// stpcpy

int strcasecmp(const char *a, const char *b);
int strncasecmp(const char *a, const char *b, size_t count);

// int strcoll(const char* a, const char* b);
// size_t strnlen(const char *str, size_t maxsize);

void itoa(char *buf, uint32_t n, int base);
int atoi(char *str);
size_t atou(const char *str);
const char *atou_return(const char *str, size_t *result);

uint32_t strcrl(string str, const char what, const char with);
uint32_t str_begins_with(string str, string with);
uint32_t str_backspace(string str, char c);
uint32_t strcount(string str, char c);

status_t strtoull(const char *s, unsigned int base, uint64_t *res);
status_t strtoll(const char *s, unsigned int base, int64_t *res);
status_t strtoul(const char *s, unsigned int base, uint32_t *res);
status_t strtol(const char *s, unsigned int base, int32_t *res);

uint16_t *utf16_to_codepoint(uint16_t *string, int *codepoint);
char *codepoint_to_utf8(int codepoint, char *stringOutput);

static bool str_has_prefix(const char *str, const char *prefix)
{
	return strncmp(str, prefix, strlen(prefix)) == 0;
}
