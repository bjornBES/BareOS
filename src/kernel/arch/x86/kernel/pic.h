/*
 * File: pic.h
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
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
    void (*send_eoi)(int irq);
    void (*mask)(int irq);
    void (*unmask)(int irq);
} pic_driver;