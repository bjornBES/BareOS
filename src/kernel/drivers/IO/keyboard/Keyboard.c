/*
 * File: Keyboard.c
 * File Created: 15 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "Keyboard.h"
#include "GKI.h"
#include "device/device.h"
#include "drivers/IO/tty/tty_types.h"

#include "kernel/memory.h"
#include <stdint.h>

#define MODULE "KB"

uint32_t keyboard_devices_max;
uint32_t keyboard_device_count;
device_t **keyboard_devices;

void update_mods(key_event *event, keyboard_priv_t *priv)
{
    key_modifiers bit = MOD_NONE;
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
                key_modifiers caps = priv->mods & MOD_CAPS;
                if (caps == 0)
                {
                    priv->mods |= bit;
                }
                else
                {
                    priv->mods |= bit;
                }
            }
            else
            {
                priv->mods &= ~bit;
            }
        }
        else
        {
            priv->mods &= ~bit;
        }
    }

    {
        key_modifiers caps = priv->mods & MOD_CAPS;
        key_modifiers shift = priv->mods & MOD_SHIFT;
        if (caps && !(shift))
        {
            priv->mods |= MOD_LSHIFT;
        }
        else if (caps && shift)
        {
            priv->mods &= ~MOD_LSHIFT; // they cancel
        }
    }
}

device_t *keyboard_get_device(uint32_t channel)
{
    for (size_t i = 0; i < keyboard_devices_max; i++)
    {
        device_t *dev = keyboard_devices[i];
        keyboard_priv_t *priv = (keyboard_priv_t *)dev->priv;
        if (priv->channel == channel)
        {
            return dev;
        }
    }
    return NULL;
}

ringbuf_index keyboard_get_input_buf(device_t *dev)
{
    // ENTER_FUNC(MODULE, "%p", dev);
    keyboard_priv_t *priv = dev->priv;
    return priv->buffer_index;
}

void keyboard_flush_input(device_t *dev)
{
    keyboard_priv_t *priv = dev->priv;
    while (ringbuf_read(priv->buffer_index, NULL, 1) != 0)
    {
    }
}

void keyboard_get_ops(device_t *dev, tty_dev_ops_t *ops)
{
    ops->get_input_buf = keyboard_get_input_buf;
    ops->flush_input = keyboard_flush_input;
}

keyboard_priv_t *keyboard_init(device_t *dev)
{
    if (keyboard_devices == NULL)
    {
        keyboard_devices_max = 8;
        keyboard_devices = calloc(8, sizeof(device_t*));
    }
    if (keyboard_device_count == keyboard_devices_max)
    {
        keyboard_devices_max <<= 1;
        keyboard_devices = realloc(keyboard_devices, sizeof(device_t*) * keyboard_devices_max);
    }
    keyboard_priv_t *keyboard = malloc(sizeof(keyboard_priv_t));
    memset(keyboard, 0, sizeof(keyboard_priv_t));
    keyboard->buffer_index = ringbuf_init(sizeof(gki_event_t), 10);
    keyboard->id = keyboard_device_count;
    keyboard_devices[keyboard_device_count] = dev;
    keyboard_device_count++;

    dev->tty_ops = keyboard_get_ops;
    return keyboard;
}
