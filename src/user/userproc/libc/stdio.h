/*
 * File: stdio.h
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include <types.h>
#include <stdarg.h>

#ifndef EOF
#define EOF (-1)
#endif

static __attribute__((unused)) fd_t const stdin = (fd_t)0;
static __attribute__((unused)) fd_t const stdout = (fd_t)1;
static __attribute__((unused)) fd_t const stderr = (fd_t)2;
static __attribute__((unused)) fd_t const stddebug = (fd_t)3;

void fputc(char c, fd_t fd);

int printf(const char *fmt, ...);
int fprintf(fd_t file, const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);
int snprintf(char *s, size_t n, const char *fmt, ...);
int vprintf(fd_t file, const char *fmt, va_list args);
int vfprintf(fd_t file, const char *fmt, va_list args);
int vsprintf(char *s, const char *fmt, va_list args);
int vsnprintf(char *s, size_t n, const char *fmt, va_list args);
