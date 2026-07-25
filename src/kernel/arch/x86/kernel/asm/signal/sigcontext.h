/*
 * File: sigcontext.h
 * File Created: 21 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#ifdef __i686__
typedef struct sigcontext
{
    uint16_t gs, __gsh;
    uint16_t fs, __fsh;
    uint16_t es, __esh;
    uint16_t ds, __dsh;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t trapno;
    uint32_t err;
    uint32_t eip;
    uint16_t cs, __csh;
    uint32_t eflags;
    uint32_t esp_at_signal;
    uint16_t ss, __ssh;
    void *fpstate;
    uint32_t oldmask;
    uint32_t cr2;
} sigcontext_t;
#else /* __x86_64__: */
typedef struct sigcontext
{
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t di;
    uint64_t si;
    uint64_t bp;
    uint64_t bx;
    uint64_t dx;
    uint64_t ax;
    uint64_t cx;
    uint64_t sp;
    uint64_t pc;
    uint64_t flags; /* RFLAGS */
    uint16_t cs;

    // dont use them... that is what Linux said... want a read?
    /*
     * Prior to 2.5.64 ("[PATCH] x86-64 updates for 2.5.64-bk3"),
     * Linux saved and restored fs and gs in these slots.  This
     * was counterproductive, as fsbase and gsbase were never
     * saved, so arch_prctl was presumably unreliable.
     *
     * These slots should never be reused without extreme caution:
     *
     *  - Some DOSEMU versions stash fs and gs in these slots manually,
     *    thus overwriting anything the kernel expects to be preserved
     *    in these slots.
     *
     *  - If these slots are ever needed for any other purpose,
     *    there is some risk that very old 64-bit binaries could get
     *    confused.  I doubt that many such binaries still work,
     *    though, since the same patch in 2.5.64 also removed the
     *    64-bit set_thread_area syscall, so it appears that there
     *    is no TLS API beyond modify_ldt that works in both pre-
     *    and post-2.5.64 kernels.
     *
     * If the kernel ever adds explicit fs, gs, fsbase, and gsbase
     * save/restore, it will most likely need to be opt-in and use
     * different context slots.
     */
    uint16_t gs;
    uint16_t fs;

    union
    {
        uint16_t ss;     /* If UC_SIGCONTEXT_SS */
        uint16_t __pad0; /* Alias name for old (!UC_SIGCONTEXT_SS) user-space */
    };

    uint64_t err;
    uint64_t trapno;
    uint64_t oldmask;
    uint64_t cr2;
    void *fpstate; /* Zero when no FPU context */
#ifdef __ILP32__
    uint32_t __fpstate_pad;
#endif
    uint64_t reserved1[8];
} sigcontext_t;
#endif /* __x86_64__ */
