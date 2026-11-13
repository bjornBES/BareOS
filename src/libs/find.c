/*
 * File: find.c
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include <binary.h>
#include <base_math.h>

#define FIND_NEXT_BIT(FETCH, MUNGE, size, start)                      \
    ({                                                                \
        unsigned long mask, idx, tmp, sz = (size), __start = (start); \
                                                                      \
        if (UNLIKELY(__start >= sz))                                  \
            goto out;                                                 \
                                                                      \
        mask = MUNGE(BITMAP_FIRST_WORD_MASK(__start));                \
        idx = __start / BITS_PER_LONG;                                \
                                                                      \
        for (tmp = (FETCH) & mask; !tmp; tmp = (FETCH))               \
        {                                                             \
            if ((idx + 1) * BITS_PER_LONG >= sz)                      \
                goto out;                                             \
            idx++;                                                    \
        }                                                             \
                                                                      \
        sz = min(idx * BITS_PER_LONG + FFS(MUNGE(tmp)), sz);          \
out:                                                                  \
        sz;                                                           \
    })

unsigned long _find_next_bit(unsigned long *addr, unsigned long nbits, unsigned long start)
{
    return FIND_NEXT_BIT(addr[idx], /* nop */, nbits, start);
}
