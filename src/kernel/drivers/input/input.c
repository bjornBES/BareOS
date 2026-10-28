/*
 * File: input.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/input/input.h"
#include "drivers/input/input_dev.h"

#include "drivers/base/class.h"

#include "ringbuf/ringbuf.h"

#include "irq/irq.h"

#include "memory.h"

#define MODULE "input"

status_t input_handler(intr_frame_t *frame, void *ctx)
{
    input_dev_t *input_dev = ctx;
    return input_dev->irq_handler(input_dev);
}

status_t input_device_added(device_t *dev)
{
    ENTER_FUNC("%p", dev);

    input_dev_t *input_dev = to_input_dev(dev);

    ringbuf_init(sizeof(key_event_t), 32, &input_dev->event_buffer);

    irq_register_handler(input_dev->irq, IRQ_SOURCE_IRQ, input_handler, input_dev, IRQ_TRIGGER_EDGE, IRQ_POLARITY_HIGH, ANY_CPU);

    return KERRNO_SUCCESSES;
}

static dev_class_t input_class = {
    .name = "input",
    .type = DEVICE_CHAR,
    .device_added = input_device_added,
};

void update_mods(key_event_t *event, input_dev_t *dev)
{
    key_modifiers_t bit = MOD_NONE;
    switch (event->key)
    {
        case KEY_LEFTSHIFT :
            bit = MOD_LSHIFT;
            break;
        case KEY_RIGHTSHIFT :
            bit = MOD_RSHIFT;
            break;
        case KEY_LEFTCTRL :
            bit = MOD_LCTRL;
            break;
        case KEY_RIGHTCTRL :
            bit = MOD_RCTRL;
            break;
        case KEY_LEFTALT :
            bit = MOD_LALT;
            break;
        case KEY_RIGHTALT :
            bit = MOD_RALT;
            break;
        case KEY_CAPSLOCK :
            bit = MOD_CAPS;
            break;
        default :
            return;
    }
    {
        if (event->action == KEY_PRESSED)
        {
            if (bit == MOD_CAPS)
            {
                key_modifiers_t caps = dev->mods & MOD_CAPS;
                if (caps == 0)
                {
                    dev->mods |= bit;
                }
                else
                {
                    dev->mods |= bit;
                }
            }
            else
            {
                dev->mods &= ~bit;
            }
        }
        else
        {
            dev->mods &= ~bit;
        }
    }

    // {
    //     key_modifiers_t caps = dev->mods & MOD_CAPS;
    //     key_modifiers_t shift = dev->mods & MOD_SHIFT;
    //     if (caps && !(shift))
    //     {
    //         dev->mods |= MOD_LSHIFT;
    //     }
    //     else if (caps && shift)
    //     {
    //         dev->mods &= ~MOD_LSHIFT; // they cancel
    //     }
    // }
}

status_t input_report_key(input_dev_t *dev, key_event_t *key)
{
    update_mods(key, dev);
    return ringbuf_write(dev->event_buffer, key);
}

status_t input_class_register()
{
    return class_register(&input_class);
}

ARCHDONE_INITCALL(input_class_register);

input_dev_t *input_create_device()
{
    ENTER_FUNC("", 0);
    input_dev_t *dev = kzmalloc(sizeof(input_dev_t));
    dev->dev.class = &input_class;
    dev_set_no_driver(&dev->dev);
    return dev;
}
