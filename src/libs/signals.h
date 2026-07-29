/*
 * File: signals.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "types.h"

#define SA_NOCLDSTOP 1
#define SA_NOCLDWAIT 2
#define SA_SIGINFO   4
#define SA_ONSTACK   0x08000000
#define SA_RESTART   0x10000000
#define SA_NODEFER   0x40000000
#define SA_RESETHAND 0x80000000
#define SA_RESTORER  0x04000000

#define SIG_DFL      ((void (*)(int))0)
#define SIG_IGN      ((void (*)(int))1)

#define SIGHUP       1  // Hangup detected on controlling terminal or death of controlling process
#define SIGINT       2  // Interrupt from keyboard
#define SIGQUIT      3  // Quit from keyboard
#define SIGILL       4  // Illegal Instruction
#define SIGTRAP      5  // Trace/breakpoint trap
#define SIGABRT      6  // Abort signal from abort(3)
#define SIGIOT       6  // IOT trap. A synonym for SIGABRT
#define SIGBUS       7  // Bus error (bad memory access)
#define SIGFPE       8  // Erroneous arithmetic operation
#define SIGKILL      9  // Kill signal
#define SIGUSR1      10 // User-defined signal 1
#define SIGSEGV      11 // Invalid memory reference
#define SIGUSR2      12 // User-defined signal 2
#define SIGPIPE      13 // Broken pipe: write to pipe with no readers; see pipe(7)
#define SIGALRM      14 // Timer signal from alarm(2)
#define SIGTERM      15 // Termination signal
#define SIGSTKFLT    16 // Stack fault on coprocessor (unused)
#define SIGCHLD      17 // Child stopped, terminated, or continued
#define SIGCONT      18 // Continue if stopped
#define SIGSTOP      19 // Stop process
#define SIGTSTP      20 // Stop typed at terminal
#define SIGTTIN      21 // Terminal input for background process
#define SIGTTOU      22 // Terminal output for background process
#define SIGURG       23 // Urgent condition on socket
#define SIGXCPU      24 // CPU time limit exceeded
#define SIGXFSZ      25 // File size limit exceeded
#define SIGVTALRM    26 // Virtual alarm clock
#define SIGPROF      27 // Profiling timer expired
#define SIGWINCH     28 // Window resize signal
#define SIGIO        29 // I/O now possible
#define SIGPWR       30 // Power failure (System V)
#define SIGSYS       31 // Bad system call

