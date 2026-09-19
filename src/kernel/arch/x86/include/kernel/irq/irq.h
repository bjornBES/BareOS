/*
 * File: irq.h
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define SAVE_IRQ()        \
    reg_t __state__ = 0;  \
    irq_save(&__state__);

#define RESTORE_IRQ()       \
    irq_restore(__state__);

void irq_arch_enable();
void irq_arch_disable();
void irq_save(reg_t *irq);
void irq_restore(reg_t irq);
