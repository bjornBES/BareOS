/*
 * File: signal.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <signals.h>

int sigaction(int signum, signal_handler handler);
int sigreturn();
int kill(pid_t pid, int sig);
