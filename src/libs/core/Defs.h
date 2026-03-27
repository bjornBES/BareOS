/*
 * File: Defs.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Mar 2026
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
#define PACKED __attribute__((packed))