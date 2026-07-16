/*
 * File: string.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define UNICODE_COMBINING_ACUTE 0x0301
#define UNICODE_COMBINING_CARON 0x030C

#define BOOL_TO_STRING ? "TRUE" : "FALSE" 

typedef char* string;

uint32_t strlen(const char* str);

char* strcpy(char* dst, char* src);
char* strncpy(char* dst, const char* src, size_t num);

char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t num);

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t num);

const char* strchr(const char* str, char chr);
char *strrchr(const char *s, int c);

// strstr

char* strtok(string str, const char* delim);
// strtok_r

// strdup
// strndup

// strerror

size_t strspn(const char* s1, const char* s2);
size_t strcspn(const char *s1, const char *s2);

// strpbrk

// stpcpy

int strcasecmp(const char *a, const char *b);
int strncasecmp(const char *a, const char *b, size_t count);

// int strcoll(const char* a, const char* b);
size_t strnlen(const char *str, size_t maxsize);

void itoa(char *buf, uint32_t n, int base);
int atoi(char* str);
size_t atou(const char* str);
const char* atou_return(const char *str, size_t* result);

uint32_t strcrl(string str, const char what, const char with);
uint32_t str_begins_with(string str, string with);
uint32_t str_backspace(string str, char c);
uint32_t strcount(string str, char c);

uint16_t* utf16_to_codepoint(uint16_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);
