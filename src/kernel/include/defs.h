/*
 * File: defs.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define ASMCALL32 __attribute__((cdecl))
#ifdef __x86_64__
#define ASMCALL 
#else
#define ASMCALL __attribute__((cdecl))
#endif
#define ARCH_CALL ASMCALL
#define PACKED __attribute__((packed))

#define inline_asm __asm__ __volatile__
