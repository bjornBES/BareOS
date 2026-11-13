/*
 * File: signals_uapi.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define SA_NOCLDSTOP      1
#define SA_NOCLDWAIT      2
#define SA_SIGINFO        4
#define SA_ONSTACK        0x08000000
#define SA_RESTART        0x10000000
#define SA_NODEFER        0x40000000
#define SA_RESETHAND      0x80000000
#define SA_RESTORER       0x04000000

#define SI_USER           0    // kill(2).
#define SI_KERNEL         128  // Sent by the kernel.
#define SI_QUEUE          (-1) // sigqueue(3).
#define SI_TIMER          (-2) // POSIX, or setitimer(2) or alarm(2) timer expired.
#define SI_MESGQ          (-3) // (since Linux 2.6.6) POSIX message queue state changed; see mq_notify(3).
#define SI_ASYNCIO        (-4) // AIO completed.
#define SI_SIGIO          (-5) // Queued SIGIO (only up to Linux 2.2. From Linux 2.4 onward SIGIO/SIGPOLL fills in si_code as described below).
#define SI_TKILL          (-6) // (since Linux 2.4.19) tkill(2) or tgkill(2).
#define SI_ASYNCNL        (-60)

#define ILL_ILLOPC        1 // Illegal opcode.
#define ILL_ILLOPN        2 // Illegal operand.
#define ILL_ILLADR        3 // Illegal addressing mode.
#define ILL_ILLTRP        4 // Illegal trap.
#define ILL_PRVOPC        5 // Privileged opcode.
#define ILL_PRVREG        6 // Privileged register.
#define ILL_COPROC        7 // Coprocessor error.
#define ILL_BADSTK        8 // Internal stack error.

#define FPE_INTDIV        1 // Integer divide by zero.
#define FPE_INTOVF        2 // Integer overflow.
#define FPE_FLTDIV        3 // Floating-point divide by zero.
#define FPE_FLTOVF        4 // Floating-point overflow.
#define FPE_FLTUND        5 // Floating-point underflow.
#define FPE_FLTRES        6 // Floating-point inexact result.
#define FPE_FLTINV        7 // Floating-point invalid operation.
#define FPE_FLTSUB        8 // Subscript out of range.

#define SEGV_MAPERR       1 // Address not mapped to object.
#define SEGV_ACCERR       2 // Invalid permissions for mapped object.
#define SEGV_BNDERR       3 // (since Linux 3.19) Failed address bound checks.
#define SEGV_PKUERR       4 // (since Linux 4.6) Access was denied by memory protection keys. The protection key which applied to this access is available via si_pkey.
#define SEGV_MTEAERR      8
#define SEGV_MTESERR      9

#define BUS_ADRALN        1 // Invalid address alignment.
#define BUS_ADRERR        2 // Nonexistent physical address.
#define BUS_OBJERR        3 // Object-specific hardware error.
#define BUS_MCEERR_AR     4 // (since Linux 2.6.32) Hardware memory error consumed on a machine check; action required.
#define BUS_MCEERR_AO     5 // (since Linux 2.6.32) Hardware memory error detected in process but not consumed; action optional.

#define TRAP_BRKPT        1 // Process breakpoint.
#define TRAP_TRACE        2 // Process trace trap.
#define TRAP_BRANCH       3 // (since Linux 2.4, IA64 only) Process taken branch trap.
#define TRAP_HWBKPT       4 // (since Linux 2.4, IA64 only) Hardware breakpoint/watchpoint.
#define TRAP_UNK          5 //

#define CLD_EXITED        1 // Child has exited.
#define CLD_KILLED        2 // Child was killed.
#define CLD_DUMPED        3 // Child terminated abnormally.
#define CLD_TRAPPED       4 // Traced child has trapped.
#define CLD_STOPPED       5 // Child has stopped.
#define CLD_CONTINUED     6 // (since Linux 2.6.9) Stopped child has continued.

#define POLL_IN           1 // Data input available.
#define POLL_OUT          2 // Output buffers available.
#define POLL_MSG          3 // Input message available.
#define POLL_ERR          4 // I/O error.
#define POLL_PRI          5 // High priority input available.
#define POLL_HUP          6 // Device disconnected.

#define SYS_SECCOMP       1 // (since Linux 3.5) Triggered by a seccomp(2) filter rule.
#define SYS_USER_DISPATCH 2

#define SIG_DFL           ((void (*)(int))0)
#define SIG_IGN           ((void (*)(int))1)

#define SIGHUP            1
#define SIGINT            2
#define SIGQUIT           3
#define SIGILL            4
#define SIGTRAP           5
#define SIGABRT           6
#define SIGIOT            6
#define SIGBUS            7
#define SIGFPE            8
#define SIGKILL           9
#define SIGUSR1           10
#define SIGSEGV           11
#define SIGUSR2           12
#define SIGPIPE           13
#define SIGALRM           14
#define SIGTERM           15
#define SIGSTKFLT         16
#define SIGCHLD           17
#define SIGCONT           18
#define SIGSTOP           19
#define SIGTSTP           20
#define SIGTTIN           21
#define SIGTTOU           22
#define SIGURG            23
#define SIGXCPU           24
#define SIGXFSZ           25
#define SIGVTALRM         26
#define SIGPROF           27
#define SIGWINCH          28
#define SIGIO             29
#define SIGPOLL           SIGIO
/*
#define SIGLOST		29
*/
#define SIGPWR            30
#define SIGSYS            31
#define SIGUNUSED         31

#define SIGRTMIN          32
#ifndef SIGRTMAX
#define SIGRTMAX _NSIG
#endif
