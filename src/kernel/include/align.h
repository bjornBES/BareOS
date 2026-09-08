/*
 * File: align.h
 * File Created: 03 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define ALIGN_DOWN(x, a)     ((x) - ((x) % (a)))
#define ALIGN_UP(x, a)       (((x) + (a) - 1) / (a) * (a))
#define ALIGN_2_DOWN(x, a)   ((x) & ~((a) - 1))
#define ALIGN_2_UP(x, a)     (((x) + (a) - 1) & ~((a) - 1))