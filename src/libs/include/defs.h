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

#define CDECL        __attribute__((cdecl))
#define ASMCALL32    __attribute__((cdecl))
#define MS_ASMCALL   __attribute__((ms_abi))
#define SYSV_ASMCALL __attribute__((sysv_abi))
#ifdef __x86_64__
#define ASMCALL
#else
#define ASMCALL __attribute__((cdecl))
#endif

#define ARCH_CALL             ASMCALL

#define PACKED                __attribute__((packed))
#define NORETURN              __attribute__((noreturn))
#define NOINLINE              __attribute__((__noinline__))
#define ALIGN(bytes)          __attribute__((aligned(bytes)))
#define SECTION(name)         __attribute__((section(name)))
#define ASSUME(var, op, expr) __attribute__((assume(var op expr)))
#define NAKED                 __attribute__((naked))
#define COLD                  __attribute__((cold))
#define USED                  __attribute__((used))
#define UNUSED                __attribute__((unused))
#define WEAK                  __attribute__((weak))
#define FORCE                 __attribute__((force))
#define ATTRIBUTE_CONST       __attribute__((__const__))
#define BITWISE               __attribute__((bitwise))
#define FALLTHROUGH           __attribute__((__fallthrough__))
#define MUST_CHECK            __attribute__((__warn_unused_result__))

#define INTERNAL              static

#define UNREACHABLE()         __builtin_unreachable()

#define INLINE                inline
#define INTERNAL_INLINE       static inline
#define FORCE_INLINE          static inline __attribute__((always_inline))
#define ALWAYS_INLINE         inline __attribute__((always_inline))

#define BTF_TYPE_TAG(value)   __attribute__((btf_type_tag(#value)))

#define __user                BTF_TYPE_TAG(user)
#define __percpu              BTF_TYPE_TAG(percpu)

#define inline_asm            __asm__ __volatile__

#define CONCAT_(a, b)         a##b
#define CONCAT(a, b)          CONCAT_(a, b)

#define STRINGIFY_(x...)      #x
#define STRINGIFY(x...)       STRINGIFY_(x)

#define LIKELY(x)             __builtin_expect(!!(x), 1)
#define UNLIKELY(x)           __builtin_expect(!!(x), 0)
#define LIKELY_NOTRACE(x)     LIKELY(x)
#define UNLIKELY_NOTRACE(x)   UNLIKELY(x)

#define WARN_ON(condition)    ({       \
    int __ret_warn_on = !!(condition); \
    UNLIKELY(__ret_warn_on);           \
})

#define WARN_ON_ONCE(condition)         WARN_ON(condition)

#define __static_assert(expr, msg, ...) _Static_assert(expr, msg)
#define static_assert(expr, ...)        __static_assert(expr, ##__VA_ARGS__, #expr)

#define ___PASTE(a, b)                  a##b
#define __PASTE(a, b)                   ___PASTE(a, b)

/* Format: __UNIQUE_ID_<name>_<__COUNTER__> */
#define __UNIQUE_ID(name)                     \
    __PASTE(__UNIQUE_ID_,                     \
            __PASTE(name,                     \
                    __PASTE(_, __COUNTER__)))

#define __ARG_PLACEHOLDER_1                                                                          0,
#define __take_second_arg(__ignored, val, ...)                                                       val

#define ____is_defined(arg1_or_junk)                                                                 __take_second_arg(arg1_or_junk 1, 0)
#define ___is_defined(val)                                                                           ____is_defined(__ARG_PLACEHOLDER_##val)
#define __is_defined(x)                                                                              ___is_defined(x)

#define IS_BUILTIN(option)                                                                           __is_defined(option)

#define IS_ENABLED(option)                                                                           IS_BUILTIN(option)

#define __COUNT_ARGS(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _n, X...) _n
#define COUNT_ARGS(X...)                                                                             __COUNT_ARGS(, ##X, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
