/*
 * File: context_arch.h
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

// switch from one thread context to another
void context_arch_switch(vaddr_t to);