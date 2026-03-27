/*
 * File: Keyboard.c
 * File Created: 13 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "Keyboard.h"
#include "libs/malloc.h"

#include "libs/memory.h"

// Set 2 base scancode -> KeyCode (index = scancode byte)
// Only need up to 0x84, everything else is KEY_UNKNOWN
static const KeyCode sc2_base[256] = {
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
    [0x12] = KEY_LSHIFT,
    [0x59] = KEY_RSHIFT,
    [0x14] = KEY_LCTRL,
    [0x11] = KEY_LALT,
    [0x58] = KEY_CAPSLOCK,
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
    [0x49] = KEY_PERIOD,
    [0x4A] = KEY_MINUS,
};

// Set 2 extended (0xE0 xx) -> KeyCode
static const KeyCode sc2_extended[256] = {
    [0x14] = KEY_RCTRL,
    [0x11] = KEY_RALT,
    [0x75] = KEY_UP,
    [0x72] = KEY_DOWN,
    [0x6B] = KEY_LEFT,
    [0x74] = KEY_RIGHT,
    [0x6C] = KEY_HOME,
    [0x69] = KEY_END,
    [0x7D] = KEY_PGUP,
    [0x7A] = KEY_PGDN,
    [0x70] = KEY_INSERT,
    [0x71] = KEY_DELETE,
};

// KeyCode -> uint32_t
static const uint32_t ascii[256] = {
    [KEY_A] = 'A',
    [KEY_B] = 'B',
    [KEY_C] = 'C',
    [KEY_D] = 'D',
    [KEY_E] = 'E',
    [KEY_F] = 'F',
    [KEY_G] = 'G',
    [KEY_H] = 'H',
    [KEY_I] = 'I',
    [KEY_J] = 'J',
    [KEY_K] = 'K',
    [KEY_L] = 'L',
    [KEY_M] = 'M',
    [KEY_N] = 'N',
    [KEY_O] = 'O',
    [KEY_P] = 'P',
    [KEY_Q] = 'Q',
    [KEY_R] = 'R',
    [KEY_S] = 'S',
    [KEY_T] = 'T',
    [KEY_U] = 'U',
    [KEY_V] = 'V',
    [KEY_W] = 'W',
    [KEY_X] = 'X',
    [KEY_Y] = 'Y',
    [KEY_Z] = 'Z',
    [KEY_0] = '0',
    [KEY_1] = '1',
    [KEY_2] = '2',
    [KEY_3] = '3',
    [KEY_4] = '4',
    [KEY_5] = '5',
    [KEY_6] = '6',
    [KEY_7] = '7',
    [KEY_8] = '8',
    [KEY_9] = '9',
    [KEY_ENTER] = '\r',
    [KEY_ESCAPE] = '\e',
    [KEY_BACKSPACE] = '\b',
    [KEY_TAB] = '\t',
    [KEY_SPACE] = ' ',
    [KEY_LSHIFT] = '\0',
    [KEY_RSHIFT] = '\0',
    [KEY_LCTRL] = '\0',
    [KEY_LALT] = '\0',
    [KEY_CAPSLOCK] = '\0',
    [KEY_F1] = '\0',
    [KEY_F2] = '\0',
    [KEY_F3] = '\0',
    [KEY_F4] = '\0',
    [KEY_F5] = '\0',
    [KEY_F6] = '\0',
    [KEY_F7] = '\0',
    [KEY_F8] = '\0',
    [KEY_F9] = '\0',
    [KEY_F10] = '\0',
    [KEY_F11] = '\0',
    [KEY_F12] = '\0',
    [KEY_COMMA] = ',',
    [KEY_PERIOD] = '.',
    [KEY_MINUS] = '-',
    [KEY_RCTRL] = '\0',
    [KEY_RALT] = '\0',
    [KEY_UP] = '\0',
    [KEY_DOWN] = '\0',
    [KEY_LEFT] = '\0',
    [KEY_RIGHT] = '\0',
    [KEY_HOME] = '\0',
    [KEY_END] = '\0',
    [KEY_PGUP] = '\0',
    [KEY_PGDN] = '\0',
    [KEY_INSERT] = '\0',
    [KEY_DELETE] = '\x7f',
};

static KeyboardState kbd_state = KBD_STATE_NORMAL;

#define BUFFER_SIZE 1024

int16_t write_pointer;
int16_t read_pointer;
key_event *key_buffer;

bool keyboard_put_key(key_event *key)
{
    if ((write_pointer + 1) == read_pointer)
    {
        return false;
    }
    key_event *pKey = &key_buffer[write_pointer];
    write_pointer++;
    if (write_pointer >= BUFFER_SIZE)
    {
        write_pointer = 0;
    }
    memcpy(pKey, key, sizeof(key_event));
    return true;
}

bool keyboard_get_key(key_event *key)
{
    if (read_pointer == write_pointer)
    {
        return false;
    }

    key_event *pKey = &key_buffer[read_pointer];
    memcpy(key, pKey, sizeof(key_event));

    read_pointer++;
    if (read_pointer >= BUFFER_SIZE)
    {
        read_pointer = 0;
    }
    return true;
}

bool keyboard_process_byte(uint8_t byte, key_event *out)
{
    switch (kbd_state)
    {
    case KBD_STATE_NORMAL:
        if (byte == 0xF0)
        {
            kbd_state = KBD_STATE_BREAK;
            return false;
        }
        if (byte == 0xE0)
        {
            kbd_state = KBD_STATE_EXTENDED;
            return false;
        }
        if (byte == 0xE1)
        { /* pause — ignore for now */
            return false;
        }
        out->key = sc2_base[byte];
        out->action = KEY_PRESSED;
        return true;

    case KBD_STATE_BREAK:
        kbd_state = KBD_STATE_NORMAL;
        out->key = sc2_base[byte];
        out->action = KEY_RELEASED;
        return true;

    case KBD_STATE_EXTENDED:
        if (byte == 0xF0)
        {
            kbd_state = KBD_STATE_EXTENDED_BREAK;
            return false;
        }
        kbd_state = KBD_STATE_NORMAL;
        out->key = sc2_extended[byte];
        out->action = KEY_PRESSED;
        return true;

    case KBD_STATE_EXTENDED_BREAK:
        kbd_state = KBD_STATE_NORMAL;
        out->key = sc2_extended[byte];
        out->action = KEY_RELEASED;
        return true;
    }
    return false;
}

uint32_t keyboard_get_ascii(key_event *key)
{
    if (key->action == KEY_PRESSED)
    {
        return 0;
    }
    return ascii[key->key];
}

void keyboard_init()
{
    key_buffer = (key_event *)calloc(BUFFER_SIZE, sizeof(key_event));
    read_pointer = 0;
    write_pointer = 0;
}
