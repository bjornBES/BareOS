/*
 * File: e9.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Mar 2026
 * Modified By: BjornBEs
 * -----
 */
#include "e9.h"

#include "libs/IO.h"
#include <core/Defs.h>

#include <stdint.h>

void e9_putc(char c)
{
    outb(0xE9, c);
}