/*
 * File: binary.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define BIT(x) (1ull << x)

#define FLAG_SET(x, flag) (x) |= (flag)
#define FLAG_UNSET(x, flag) (x) &= ~(flag)
#define FLAG_IS_SET(x, flag) (((x) & (flag)) == (flag))
#define FLAG_GET(x, flag) (x) & (flag)

#define BIT_SET(x, bit) (x) |= (1 << (bit))
#define BIT_UNSET(x, bit) (x) &= ~(1 << (bit))
#define BIT_GET(x, bit) (((x) >> (bit)) & 1)