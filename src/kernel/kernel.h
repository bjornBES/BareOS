/*
 * File: main.h
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define RETURN_ERROR   -1
#define RETURN_FAILED   0
#define RETURN_GOOD     1

void panic(char *module, char *file, int line, char* message, ...);

