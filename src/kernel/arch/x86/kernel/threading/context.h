/*
 * File: context.h
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/registers.h"
#include <types.h>

typedef struct
{
    reg_t stack_pointer;
    registers regs;
} context;

void context_switch(reg_t new);
