/*
 * File: generic_stdlib.c
 * File Created: 17 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "generic/generic_stdlib.h"

#include <stdint.h>
#include <types.h>
#include <defs.h>
#include <overflow.h>
#include <kerrno_uapi.h>
#include <uapi_ctype.h>

#define KSTRTOX_OVERFLOW (1u << 31)

const char *parse_integer_fixup_radix(const char *s, unsigned int *base)
{
    if (*base == 0)
    {
        if (s[0] == '0')
        {
            if (tolower(s[1]) == 'x' && isxdigit(s[2]))
            {
                *base = 16;
            }
            else
            {
                *base = 8;
            }
        }
        else
        {
            *base = 10;
        }
    }
    if (*base == 16 && s[0] == '0' && tolower(s[1]) == 'x')
    {
        s += 2;
    }
    return s;
}

unsigned int _parse_integer_limit(const char *s, unsigned int base, uint64_t *p, size_t max_chars, unsigned long long init)
{
    unsigned int rv, overflow = 0;
    unsigned long long res;

    res = init;
    for (rv = 0; rv < max_chars; rv++, s++)
    {
        unsigned int c = *s;
        unsigned int lc = tolower(c);
        unsigned int val;

        if ('0' <= c && c <= '9')
        {
            val = c - '0';
        }
        else if ('a' <= lc && lc <= 'f')
        {
            val = lc - 'a' + 10;
        }
        else
        {
            break;
        }

        if (val >= base)
        {
            break;
        }

        if (UNLIKELY(res & (~0ull << 60)))
        {
            if (check_mul_overflow(res, base, &res) ||
                check_add_overflow(res, val, &res))
            {
                res = __UINT64_MAX__;
                overflow = KSTRTOX_OVERFLOW;
            }
        }
        else
        {
            res = res * base + val;
        }
    }
    *p = res;
    return rv | overflow;
}

#define _parse_integer0(s, base, res, ...)             \
    _parse_integer_limit(s, base, res, __INT_MAX__, 0)

#define _parse_integer1(s, base, res, max_chars, ...) \
    _parse_integer_limit(s, base, res, max_chars, 0)

#define _parse_integer2(s, base, res, max_chars, init, ...) \
    _parse_integer_limit(s, base, res, max_chars, init)

#define _parse_integer(s, base, res, ...)                                      \
    CONCAT(_parse_integer, COUNT_ARGS(__VA_ARGS__))(s, base, res, __VA_ARGS__)

status_t _strtoull(const char *s, unsigned int base, uint64_t *out_ret)
{
    uint64_t _res;

    uint32_t rv = _parse_integer(s, base, &_res);

    if (rv & KSTRTOX_OVERFLOW)
    {
        return KERRNO_RESULT_NOT_REPRESENTABLE;
    }
    if (rv == 0)
    {
        return KERRNO_BAD_VALUE;
    }
    s += rv;
    if (*s == '\n')
    {
        s++;
    }
    if (*s)
    {
        return KERRNO_BAD_VALUE;
    }
    *out_ret = _res;
    return KERRNO_SUCCESS;
}

WEAK status_t strtoull(const char *s, unsigned int base, uint64_t *res)
{
    if (s[0] == '+')
    {
        s++;
    }
    return _strtoull(s, base, res);
}

WEAK status_t strtoll(const char *s, unsigned int base, int64_t *res)
{
    uint64_t tmp;
    status_t rv;

    if (s[0] == '-')
    {
        rv = _strtoull(s + 1, base, &tmp);
        if (rv < 0)
        {
            return rv;
        }
        if ((long long)-tmp > 0)
        {
            return KERRNO_RESULT_NOT_REPRESENTABLE;
        }
        *res = -tmp;
    }
    else
    {
        rv = strtoull(s, base, &tmp);
        if (rv < 0)
        {
            return rv;
        }
        if ((long long)tmp < 0)
        {
            return KERRNO_RESULT_NOT_REPRESENTABLE;
        }
        *res = tmp;
    }
    return 0;
}

status_t _strtoul(const char *s, unsigned int base, uint32_t *res)
{
    uint64_t tmp;
    status_t rv;

    rv = strtoull(s, base, &tmp);
    if (rv < 0)
    {
        return rv;
    }
    if (tmp != (unsigned long)tmp)
    {
        return KERRNO_RESULT_NOT_REPRESENTABLE;
    }
    *res = tmp;
    return 0;
}

status_t _strtol(const char *s, unsigned int base, int32_t *res)
{
    int64_t tmp;
    status_t rv;

    rv = strtoll(s, base, &tmp);
    if (rv < 0)
    {
        return rv;
    }
    if (tmp != (long)tmp)
    {
        return KERRNO_RESULT_NOT_REPRESENTABLE;
    }
    *res = tmp;
    return 0;
}

WEAK status_t strtoul(const char *s, unsigned int base, uint32_t *res)
{
    uint64_t tmp;
    int rv;

    rv = strtoull(s, base, &tmp);
    if (rv < 0)
    {
        return rv;
    }
    if (tmp != (uint32_t)tmp)
    {
        return KERRNO_RESULT_NOT_REPRESENTABLE;
    }
    *res = tmp;
    return 0;
}

WEAK status_t strtol(const char *s, unsigned int base, int32_t *res)
{
    int64_t tmp;
    int rv;

    rv = strtoll(s, base, &tmp);
    if (rv < 0)
    {
        return rv;
    }
    if (tmp != (int)tmp)
    {
        return KERRNO_RESULT_NOT_REPRESENTABLE;
    }
    *res = tmp;
    return 0;
}
