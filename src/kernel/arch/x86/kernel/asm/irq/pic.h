/*
 * File: pic.h
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const char* name;
    bool (*probe)();
    void (*initialize)(uint8_t offsetPic1, uint8_t offsetPic2, bool autoEoi);
    void (*disable)();
    void (*send_eoi)(uint32_t irq);
    void (*mask)(uint32_t irq);
    void (*unmask)(uint32_t irq);
    uint32_t (*is_masked)(uint32_t irq);
    uint32_t (*get_real_irq)(uint32_t irq);
} pic_driver;