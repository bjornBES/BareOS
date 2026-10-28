/*
 * File: input_dev.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "input_codes.h"

#include "dev/devicet.h"

#include <types.h>
#include <binary.h>

#define LED_NUM_LOCK     0x00
#define LED_CAPS_LOCK    0x01
#define LED_SCROLL_LOCK  0x02
#define LED_COMPOSE  0x03
#define LED_KANA     0x04
#define LED_SLEEP    0x05
#define LED_SUSPEND  0x06
#define LED_MUTE     0x07
#define LED_MISC     0x08
#define LED_MAIL     0x09
#define LED_CHARGING 0x0A
#define LED_MAX      0x0F
#define LED_CNT      (LED_MAX + 1)

typedef enum input_event_type
{
    EV_KEY,
    EV_LED,

    EV_CNT

} input_event_type_t;

typedef struct input_dev input_dev_t;

typedef struct input_dev
{
    kernel_irq_t irq;

    key_modifiers_t mods;

    unsigned long evbit[BITS_TO_LONGS(EV_CNT)];
    unsigned long keybit[BITS_TO_LONGS(KEY_CNT)];
    // unsigned long relbit[BITS_TO_LONGS(REL_CNT)];
    // unsigned long absbit[BITS_TO_LONGS(ABS_CNT)];
    // unsigned long mscbit[BITS_TO_LONGS(MSC_CNT)];
    unsigned long ledbit[BITS_TO_LONGS(LED_CNT)];
    // unsigned long sndbit[BITS_TO_LONGS(SND_CNT)];
    // unsigned long ffbit[BITS_TO_LONGS(FF_CNT)];
    // unsigned long swbit[BITS_TO_LONGS(SW_CNT)];

    status_t (*flush)(input_dev_t *dev);
    status_t (*event)(input_dev_t *dev, input_event_type_t type, unsigned int code, int value);

    status_t (*irq_handler)(input_dev_t *dev);

    device_t dev;
} input_dev_t;

#define to_input_dev(n) container_of(n, input_dev_t, dev)
