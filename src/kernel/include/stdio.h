/*
 * File: stdio.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "kernel/io.h"
#include "VFS/vfs.h"
#include "kernel.h"

#include <stdarg.h>

#ifndef EOF
#define EOF (-1)
#endif

static __attribute__((unused)) fd_t const stdin = (fd_t)VFS_FD_STDIN;
static __attribute__((unused)) fd_t const stdout = (fd_t)VFS_FD_STDOUT;
static __attribute__((unused)) fd_t const stderr = (fd_t)VFS_FD_STDERR;
static __attribute__((unused)) fd_t const stddebug = (fd_t)VFS_FD_DEBUG;

#define KernelPanic(module, ...) panic(module, __FILE__, __LINE__, __VA_ARGS__)

void vga_clear();

int fputc(char c, fd_t file);
int fputs(const char *s, fd_t file);

int putc(char c, fd_t file);
int putchar(int c);
int puts(const char *s);

// file shit

fd_t fopen(char *filename, const char *mode);

// fd_t fdopen(int fildes, const char *mode);
// fd_t popen(const char *command, const char *mode);
// fd_t freopen(const char *filename, const char *mode, fd_t file);

int fclose(fd_t file);

// int pclose(fd_t file);

int fileno(fd_t file);
int feof(fd_t file);
int ferror(fd_t file);
int fflush(fd_t file);

int fseek(fd_t file, int64_t offset, int whence);
int fseeko(fd_t file, off_t offset, int whence);

int64_t ftell(fd_t stream);
off_t ftello(fd_t stream);

size_t fread(void *buffer, size_t size, size_t count, fd_t stream);

size_t fwrite(const void *buffer, size_t size, size_t count, fd_t stream);

int printf(const char *fmt, ...);
int fprintf(fd_t file, const char *fmt, ...);
int sprintf(char *s, const char *format, ...);
int snprintf(char *s, size_t n, const char *format, ...);
int vprintf(fd_t file, const char *format, va_list args);
int vfprintf(fd_t file, const char *fmt, va_list args);
int vsprintf(char *s, const char *format, va_list args);
int vsnprintf(char *s, size_t n, const char *format, va_list args);

void panic(char *module, char *file, int line, char *message, ...);

/* 
int fsetpos(fd_t, const fpos_t *);
int fgetpos(fd_t, fpos_t *);

char *ctermid(char *);
char *cuserid(char *)(LEGACY);

void clearerr(fd_t);
int fgetc(fd_t);
char *fgets(char *, int, fd_t);
void flockfile(fd_t);
int fscanf(fd_t, const char *, ...);
int ftrylockfile(fd_t);
void funlockfile(fd_t);
int getc(fd_t);
int getchar(void);
int getc_unlocked(fd_t);
int getchar_unlocked(void);
int getopt(int, char *const[], const char)(LEGACY);
char *gets(char *);
int getw(fd_t);
void perror(const char *);
int putc_unlocked(int, fd_t);
int putchar_unlocked(int);
int putw(int, fd_t);
int remove(const char *);
int rename(const char *, const char *);
void rewind(fd_t);
int scanf(const char *, ...);
void setbuf(fd_t, char *);
int setvbuf(fd_t, char *, int, size_t);
int sscanf(const char *, const char *, int...);
char *tempnam(const char *, const char *);
fd_t tmpfile(void);
char *tmpnam(char *);
int ungetc(int, fd_t);
 */