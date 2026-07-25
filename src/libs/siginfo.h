/*
 * File: siginfo.h
 * File Created: 21 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef union sigval
{
    int sival_int;
    void *sival_ptr;
} sigval_t;

typedef struct
{
    int32_t si_signo; /* Signal number */
    int32_t si_errno; /* An errno value */
    int32_t si_code;  /* Signal code */

    union
    {
        char __pad[128 - 2 * sizeof(int) - sizeof(long)];

        struct
        {

            union
            {

                struct
                {
                    pid_t si_pid; /* Sending process ID */
                    uid_t si_uid; /* Real user ID of sending process */
                };

                struct
                {
                    int32_t si_timerid; /* Timer ID; POSIX.1b timers */
                    int32_t si_overrun; /* Timer overrun count; POSIX.1b timers */
                };
            };

            union
            {
                sigval_t si_value; /* Signal value */

                struct
                {
                    int32_t si_status; /* Exit value or signal */
                    clock_t si_utime;  /* User time consumed */
                    clock_t si_stime;  /* System time consumed */
                };
            };
        } si_common;

        struct
        {

            void *si_addr;       /* Memory location which caused fault */
            int16_t si_addr_lsb; /* Least significant bit of address (since Linux 2.6.32) */

            union
            {
                struct
                {
                    void *si_lower; /* Lower bound when address violation occurred (since Linux 3.19) */
                    void *si_upper; /* Upper bound when address violation occurred (since Linux 3.19) */
                };

                int32_t si_pkey; /* Protection key on PTE that caused fault (since Linux 4.6) */
            };
        } sigfault;

        struct
        {
            int64_t si_band; /* Band event (was int in glibc 2.3.2 and earlier) */
            fd_t si_fd;      /* File descriptor */
        } sigpoll;

        struct
        {
            void *si_call_addr; /* Address of system call instruction (since Linux 3.5) */
            int32_t si_syscall; /* Number of attempted system call (since Linux 3.5) */
            uint32_t si_arch;   /* Architecture of attempted system call (since Linux 3.5) */
        } sigsys;
    };
} siginfo_t;