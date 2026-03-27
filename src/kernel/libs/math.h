/*
 * File: math.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#define abs(a) (((a) < 0)?-(a):(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define sign(x) ((x < 0) ? -1 :((x > 0) ? 1 : 0))
