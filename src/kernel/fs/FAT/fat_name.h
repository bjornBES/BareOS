/*
 * File: fat_name.h
 * File Created: 13 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

int fat_83_to_name(const char raw83[11], char out[13]);
int fat_name_to_83(const char *name, uint8_t out83[11]);

