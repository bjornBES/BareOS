/*
 * File: i686.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <core/Defs.h>
#include <stdint.h>
#include <stdio.h>
#include "debug/debug.h"

#define Breakpoint() __asm__ __volatile__("int3" : : : "memory")

void ASMCALL i686_Panic();
