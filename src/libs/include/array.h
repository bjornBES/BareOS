/*
 * File: array.h
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#define ARRAY_SIZE(array) ((size_t)(sizeof(array) / sizeof(array[0])))