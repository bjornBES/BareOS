/*
 * File: generic_stdlib.h
 * File Created: 17 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <types.h>

status_t strtoull(const char *s, unsigned int base, uint64_t *res);
status_t strtoll(const char *s, unsigned int base, int64_t *res);
status_t strtoul(const char *s, unsigned int base, uint32_t *res);
status_t strtol(const char *s, unsigned int base, int32_t *res);