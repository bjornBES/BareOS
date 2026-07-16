/*
 * File: I8042_keyboard.c
 * File Created: 21 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "I8042_keyboard.h"
#include "drivers/IO/keyboard/Keyboard.h"
#include "drivers/IO/keyboard/GKI.h"
#include "kernel.h"
#include "I8042.h"
#include "I8042_io.h"
#include "kernel/irq.h"
#include "kernel/memory.h"

#include "debug/debug.h"
#define MODULE "I8042-KBD"

typedef enum
{
    I8042_KBD_STATE_NORMAL,
    I8042_KBD_STATE_BREAK,          // saw 0xF0
    I8042_KBD_STATE_EXTENDED,       // saw 0xE0
    I8042_KBD_STATE_EXTENDED_BREAK, // saw 0xE0 0xF0
} i8042_kb_state;

// Set 2 base scancode -> key_code (index = scancode byte)
// Only need up to 0x84, everything else is KEY_UNKNOWN
static const key_code sc2_base[256] = {
    [0x1C] = KEY_A,
    [0x32] = KEY_B,
    [0x21] = KEY_C,
    [0x23] = KEY_D,
    [0x24] = KEY_E,
    [0x2B] = KEY_F,
    [0x34] = KEY_G,
    [0x33] = KEY_H,
    [0x43] = KEY_I,
    [0x3B] = KEY_J,
    [0x42] = KEY_K,
    [0x4B] = KEY_L,
    [0x3A] = KEY_M,
    [0x31] = KEY_N,
    [0x44] = KEY_O,
    [0x4D] = KEY_P,
    [0x15] = KEY_Q,
    [0x2D] = KEY_R,
    [0x1B] = KEY_S,
    [0x2C] = KEY_T,
    [0x3C] = KEY_U,
    [0x2A] = KEY_V,
    [0x1D] = KEY_W,
    [0x22] = KEY_X,
    [0x35] = KEY_Y,
    [0x1A] = KEY_Z,
    [0x45] = KEY_0,
    [0x16] = KEY_1,
    [0x1E] = KEY_2,
    [0x26] = KEY_3,
    [0x25] = KEY_4,
    [0x2E] = KEY_5,
    [0x36] = KEY_6,
    [0x3D] = KEY_7,
    [0x3E] = KEY_8,
    [0x46] = KEY_9,
    [0x5A] = KEY_ENTER,
    [0x76] = KEY_ESCAPE,
    [0x66] = KEY_BACKSPACE,
    [0x0D] = KEY_TAB,
    [0x29] = KEY_SPACE,
    [0x12] = KEY_LEFTSHIFT,
    [0x59] = KEY_RIGHTSHIFT,
    [0x14] = KEY_LEFTCTRL,
    [0x11] = KEY_LEFTALT,
    [0x05] = KEY_F1,
    [0x06] = KEY_F2,
    [0x04] = KEY_F3,
    [0x0C] = KEY_F4,
    [0x03] = KEY_F5,
    [0x0B] = KEY_F6,
    [0x83] = KEY_F7,
    [0x0A] = KEY_F8,
    [0x01] = KEY_F9,
    [0x09] = KEY_F10,
    [0x78] = KEY_F11,
    [0x07] = KEY_F12,
    [0x41] = KEY_COMMA,
    [0x49] = KEY_DOT,
    [0x4A] = KEY_MINUS,
    [0x7E] = KEY_SCROLL_LOCK,
    [0x58] = KEY_CAPSLOCK,
};

// Set 2 extended (0xE0 xx) -> key_code
static const key_code sc2_extended[256] = {
    [0x14] = KEY_RIGHTCTRL,
    [0x11] = KEY_RIGHTALT,
    [0x75] = KEY_UP,
    [0x72] = KEY_DOWN,
    [0x6B] = KEY_LEFT,
    [0x74] = KEY_RIGHT,
    [0x6C] = KEY_HOME,
    [0x69] = KEY_END,
    [0x7D] = KEY_PAGEUP,
    [0x7A] = KEY_PAGEDOWN,
    [0x70] = KEY_INSERT,
    [0x71] = KEY_DELETE,
};

static i8042_kb_state i8042_state = I8042_KBD_STATE_NORMAL;
spinlock_t i8042_handler;

bool i8042_process_byte(uint8_t byte, key_event *out)
{
    switch (i8042_state)
    {
        case I8042_KBD_STATE_NORMAL :
            if (byte == 0xF0)
            {
                i8042_state = I8042_KBD_STATE_BREAK;
                return false;
            }
            if (byte == 0xE0)
            {
                i8042_state = I8042_KBD_STATE_EXTENDED;
                return false;
            }
            if (byte == 0xE1)
            {
                return false;
            }
            out->key = sc2_base[byte];
            out->action = KEY_PRESSED;
            return true;

        case I8042_KBD_STATE_BREAK :
            i8042_state = I8042_KBD_STATE_NORMAL;
            out->key = sc2_base[byte];
            out->action = KEY_RELEASED;
            return true;

        case I8042_KBD_STATE_EXTENDED :
            if (byte == 0xF0)
            {
                i8042_state = I8042_KBD_STATE_EXTENDED_BREAK;
                return false;
            }
            i8042_state = I8042_KBD_STATE_NORMAL;
            out->key = sc2_extended[byte];
            out->action = KEY_PRESSED;
            return true;

        case I8042_KBD_STATE_EXTENDED_BREAK :
            i8042_state = I8042_KBD_STATE_NORMAL;
            out->key = sc2_extended[byte];
            out->action = KEY_RELEASED;
            return true;
    }
    return false;
}

void I8042_first_channel_handler(device_t *dev)
{
    // ENTER_FUNC(MODULE, "%p", dev);
    spinlock_acquire(&i8042_handler);
    // read data
    uint8_t raw = I8042_read(I8042_DATA_PORT);
    
    log_debug(MODULE, "I8042_first_channel_handler: data = 0x%X", raw);
    
    // process data
    if (I8042_first_channel_device == I8042_CHANNEL_KEYBOARD_INITALIZED)
    {
        key_event ev;
        if (i8042_process_byte(raw, &ev))
        {
            keyboard_priv_t *priv = (keyboard_priv_t *)dev->priv;
            log_debug(MODULE, "I8042_first_channel_handler: priv = %p", priv);
            gki_event_t event;
            update_mods(&ev, priv);
            if (ev.action == KEY_PRESSED)
            {
                gki_process_key_event(&ev, &event);
                ringbuf_write(priv->buffer_index, &event);
            }
        }
    }
    I8042_first_channel_buffer_pointer = 0;
    spinlock_release(&i8042_handler);
}

void i8042_set_leds(device_t *dev, uint8_t leds)
{
    keyboard_priv_t *priv = (keyboard_priv_t *)dev->priv;
    priv->led_state = leds;

    I8042_write_cmd(0xED);
    I8042_write(I8042_DATA_PORT, leds & 0x07);
}

size_t kbd_read(void *buffer, off_t offset, size_t count, device_t *dev)
{
    keyboard_priv_t *priv = (keyboard_priv_t *)dev->priv;
    return ringbuf_read(priv->buffer_index, buffer, count) * count;
}

device_t *I8042_keyboard_device()
{
    device_t *kb = malloc(sizeof(device_t));
    memset(kb, 0, sizeof(device_t));
    kb->type = DEVICE_CHAR;
    kb->flags = DEVICE_FLAG_READABLE;
    kb->class_name = "kbd";
    kb->read = kbd_read;

    keyboard_priv_t *priv = keyboard_init(kb);
    priv->set_leds = i8042_set_leds;
    priv->channel = 1;
    kb->priv = priv;


    return kb;
}
