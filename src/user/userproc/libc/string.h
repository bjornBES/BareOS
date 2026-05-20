/*
 * File: string.h
 * File Created: 17 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

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

char* strtok(char *str, const char* delim);
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