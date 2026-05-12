/*
 * File: stdio.h
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include <printf_driver/printf.h>
#include <types.h>

#ifndef EOF
#define EOF (-1)
#endif

static __attribute__((unused)) fd_t const stdin = (fd_t)0;
static __attribute__((unused)) fd_t const stdout = (fd_t)1;
static __attribute__((unused)) fd_t const stderr = (fd_t)2;
static __attribute__((unused)) fd_t const stddebug = (fd_t)3;

void fputc(char c, fd_t fd);

int fprintf(fd_t file, const char *fmt, ...);
int vfprintf(fd_t file, const char *fmt, va_list args);
int printf(const char *fmt, ...);
int snprintf(char *s, size_t n, const char *format, ...);
int sprintf(char *s, const char *format, ...);
