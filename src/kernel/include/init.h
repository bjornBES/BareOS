/*
 * File: init.h
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>

/* sections */

// These is for kernel start up until an arch calls kernel_main in main.c
#define __init		SECTION(".init.text") COLD