/*
 * File: def.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define ASMCALL32  __attribute__((cdecl))
#define MS_ASMCALL __attribute__((ms_abi))
#ifdef __x86_64__
#define ASMCALL
#else
#define ASMCALL __attribute__((cdecl))
#endif

#define ARCH_CALL             ASMCALL

#define PACKED                __attribute__((packed))
#define NORETURN              __attribute__((noreturn))
#define ALIGN(bytes)          __attribute__((aligned(bytes)))
#define SECTION(name)         __attribute__((section(name)))
#define ASSUME(var, op, expr) __attribute__((assume(var op expr)))
#define NAKED                 __attribute__((naked))
#define COLD                  __attribute__((cold))
#define USED                  __attribute__((used))
#define UNUSED                __attribute__((unused))

#define INTERNAL

#define INLINE                static inline
#define ALWAYS_INLINE         __attribute__((always_inline))

#define BTF_TYPE_TAG(value)   __attribute__((btf_type_tag(#value)))

#define __user                BTF_TYPE_TAG(user)
#define __percpu              BTF_TYPE_TAG(percpu)

#define inline_asm            __asm__ __volatile__