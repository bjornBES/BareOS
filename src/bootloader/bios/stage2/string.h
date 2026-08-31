/*
 * File: string.h
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <uapi_string.h>

int16_t *utf16_to_codepoint(int16_t *string, int *codepoint);

char *codepoint_to_utf8(int codepoint, char *stringOutput);