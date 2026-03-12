/*
 * File: stdio.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <IO.h>
#include <kernel_core/vfsBase.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifndef EOF
#define EOF (-1)
#endif

static __attribute__((unused)) fd_t const stdin = (fd_t)VFS_FD_STDIN;
static __attribute__((unused)) fd_t const stdout = (fd_t)VFS_FD_STDOUT;
static __attribute__((unused)) fd_t const stderr = (fd_t)VFS_FD_STDERR;
static __attribute__((unused)) fd_t const stddebug = (fd_t)VFS_FD_DEBUG;

#define KernelPanic(module, ...) panic(module, __FILE__, __LINE__, __VA_ARGS__)

void vga_clear();

void fputc(char c, fd_t file);
void fputs(const char* s, fd_t file);

void putc(char c);
void puts(const char* s);

int fprintf(fd_t file, const char *fmt, ...);
int vfprintf(fd_t file, const char *fmt, va_list args);
int printf(const char *fmt, ...);
int snprintf(char *s, size_t n, const char *format, ...);
int sprintf(char *s, const char *format, ...);

void panic(char* module, char* file, int line, char* message, ...);
