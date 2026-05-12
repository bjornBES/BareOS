/*
 * File: unistd.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <pledge_types.h>

size_t read(fd_t fd, void *buf, size_t len);
size_t write(fd_t fd, const void *buf, size_t len);
pid_t getpid();
pid_t fork();
void pledge(uint32_t pledges);
int execve(const char *filename, const char *argv[], const char *envp[]);
void _exit(int status);
