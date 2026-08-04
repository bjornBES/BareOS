/*
 * File: tss.h
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "kernel/asm/segment/gdt.h"
#include "tss_type.h"


void tss_load(uint16_t segment_sel);
void tss_initialize(tss_t *tss, gdt_entry_t *descriptor);
