#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <kernel/vfs.h>

static __attribute__((unused)) fd_t const stdin = (fd_t)VFS_FD_STDIN;
static __attribute__((unused)) fd_t const stdout = (fd_t)VFS_FD_STDOUT;
static __attribute__((unused)) fd_t const stderr = (fd_t)VFS_FD_STDERR;
static __attribute__((unused)) fd_t const stddebug = (fd_t)VFS_FD_DEBUG;

#ifndef EOF
#define EOF (-1)
#endif

int fprintf(fd_t file, const char *fmt, ...);
int vfprintf(fd_t file, const char *fmt, va_list args);
int printf(const char *fmt, ...);
int snprintf(char *s, size_t n, const char *format, ...);
int sprintf(char *s, const char *format, ...);
