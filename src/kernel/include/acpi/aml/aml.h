/*
 * File: aml.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "aml_types.h"

#define SLP_EN 0x2000

void aml_init(void *dsdt, uint32_t length);
int aml_eval(const char *path, aml_val_t *result);
void aml_notify(uint32_t event);
