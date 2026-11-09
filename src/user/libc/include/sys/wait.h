/*
 * File: wait.h
 * File Created: 10 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <types.h>


pid_t waitpid(pid_t pid, int *wstatus, int options);