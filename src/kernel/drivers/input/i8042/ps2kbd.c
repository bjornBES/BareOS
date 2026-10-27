/*
 * File: ps2kbd.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "i8042.h"

#include "dev/device.h"

#include "drivers/input/input.h"
#include "drivers/input/input_codes.h"

#include "debug/debug.h"

#include "irq/irq.h"

#include "ringbuf/ringbuf.h"

#include "memory.h"

#include <binary.h>

#define MODULE "ps2-kbd"

// Set 2 base scancode -> key_code_t (index = scancode byte)
// Only need up to 0x84, everything else is KEY_UNKNOWN
static const key_code_t sc2_base[256] = {
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

// Set 2 extended (0xE0 xx) -> key_code_t
static const key_code_t sc2_extended[256] = {
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

typedef enum
{
    I8042_KBD_STATE_NORMAL,
    I8042_KBD_STATE_BREAK,          // saw 0xF0
    I8042_KBD_STATE_EXTENDED,       // saw 0xE0
    I8042_KBD_STATE_EXTENDED_BREAK, // saw 0xE0 0xF0
} i8042_kb_state_t;

static i8042_kb_state_t i8042_state = I8042_KBD_STATE_NORMAL;
spinlock_t i8042_handler;

bool i8042_process_byte(uint8_t byte, key_event_t *out)
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

status_t i8042_kbd_handler(input_dev_t *dev)
{
    i8042_port_t *port = dev->dev.drv_data;
    if (!port->driver_bound)
    {
        KERRNO_RETURN(KERRNO_DEV_NOT_READY, "");
    }

    uint8_t status = i8042_read(I8042_STATUS_REG);
    if (BIT_GET(status, I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
    {
        return KERRNO_SUCCESSES;
    }

    uint8_t raw = i8042_read(I8042_DATA_REG);

    // process data
    key_event_t ev;
    memset(&ev, 0, sizeof(key_event_t));
    if (i8042_process_byte(raw, &ev))
    {
        input_report_key(dev, &ev);
    }
    return KERRNO_SUCCESSES;
}

status_t i8042_kbd_set_leds(input_dev_t *dev)
{
    uint8_t led_status = TEST_BIT(LED_SCROLL_LOCK, dev->ledbit) ? BIT(0) : 0;
    led_status |= TEST_BIT(LED_NUM_LOCK, dev->ledbit) ? BIT(1) : 0;
    led_status |= TEST_BIT(LED_CAPS_LOCK, dev->ledbit) ? BIT(2) : 0;
    led_status |= TEST_BIT(LED_KANA, dev->ledbit) ? BIT(4) : 0;

    i8042_write_cmd(0xED);
    i8042_write(I8042_DATA_REG, led_status);
    return KERRNO_SUCCESSES;
}

status_t i8042_kbd_probe(i8042_port_t *port)
{
    ENTER_FUNC("%p", port);
    // step 1: Disable devices
    i8042_write_cmd(0xAD);

    // step 2: Flush The Output Buffer
    i8042_flush_output_buffer(1000);

    // step 3: Set the Controller Configuration Byte
    uint8_t conf = i8042_read_controller_conf();
    BIT_UNSET(conf, 0); // Disable first PS/2 port interrupts
    BIT_UNSET(conf, 6); // Disable first PS/2 port translation
    BIT_UNSET(conf, 4); // Enable first PS/2 port clock

    BIT_UNSET(conf, 1); // Disable second PS/2 port interrupts
    i8042_write_controller_conf(conf);

    // step 4: Perform Controller Self Test
    i8042_write_cmd(0xAA);
    if (i8042_read(I8042_DATA_REG) != 0x55)
    {
        trace_err(MODULE, "self test failed");
        return KERRNO_UNSUCCESS;
    }

    // step 5: Determine If There Are 2 Channels
    port->exists = true;

    // step 6: Perform Interface Tests + Enable Interrupts
    {
        i8042_write_cmd(0xAB);
        if (i8042_read(I8042_DATA_REG) != 0)
        {
            KERRNO_RETURN(KERRNO_BAD_VALUE, "failed interface test for first channel");
        }
    }

    conf = i8042_read_controller_conf();
    {
        BIT_UNSET(conf, 0); // disable interrupts
        // irq_arch_register(1, I8042_keyboard_handler, NULL);
        // handlers
    }
    i8042_write_controller_conf(conf);

    // step 7: Enable Devices
    {
        i8042_write_cmd(0xAE);
        i8042_write_first_channel(0xFF); // reset
        if (i8042_first_channel_wait_for_ack() != KERRNO_SUCCESSES)
        {
            KERRNO_RETURN(KERRNO_DEV_TIMEOUT, "First channel no ACK after reset");
        }
        if (i8042_first_channel_wait_for_reset() != KERRNO_SUCCESSES)
        {
            KERRNO_RETURN(KERRNO_DEV_TIMEOUT, "First channel no response after reset");
        }
    }

    conf = i8042_read_controller_conf();

    if (BIT_GET(i8042_read(I8042_STATUS_REG), I8042_STAT_OUTPUT_BUFF_STATE))
    {
        KERRNO_RETURN(KERRNO_TIMED_OUT, "Couldn't flush Output buffer");
    }
    // check what type of device is connected on first channel
    {
        // after reset device should not be in streaming mode, but to be sure disable streaming
        i8042_write_first_channel(0xF5);
        if (i8042_first_channel_wait_for_ack() != KERRNO_SUCCESSES)
        {
            KERRNO_RETURN(KERRNO_DEV_TIMEOUT, "First channel no ACK after 0xF5");
        }

        // read device ID
        i8042_write_first_channel(0xF2);
        if (i8042_first_channel_wait_for_ack() == KERRNO_SUCCESSES)
        {
            if (true)
            {
                uint8_t data = i8042_read(I8042_DATA_REG);
                uint8_t data2 = 0;

                // Some devices send a second ID byte, not always.
                if (i8042_first_channel_wait_for_response())
                {
                    data2 = i8042_read(I8042_DATA_REG);
                }

                if (data == 0xAA || data == 0xAB || data == 0xAC)
                {
                    // I8042_first_channel_device = I8042_CHANNEL_KEYBOARD_CONNECTED;
                }
                else
                {
                    // I8042_first_channel_device = data; // unknown device
                }

                i8042_write_first_channel(0xF4);
                if (i8042_first_channel_wait_for_ack() == KERRNO_SUCCESSES)
                {
                    port->driver_bound = true;
                }
                else
                {
                    KERRNO_RETURN(KERRNO_DEV_TIMEOUT, "First channel keyboard not ACKed");
                }

                // enable interrupts
                if (port->exists == true)
                {
                    conf |= (1 << 0); // enable interrupts
                }

                trace_debug(MODULE, "First channel device ID: %x %x", data, data2);
            }
            else
            {
                KERRNO_RETURN(KERRNO_DEV_ID_ERROR, "First channel device did not send ID data");
            }
        }
        else
        {
            KERRNO_RETURN(KERRNO_DEV_ID_ERROR, "First channel device did not send ID ack");
        }
    }

    conf = 0x23;

    i8042_write_controller_conf(conf);

    input_dev_t *input_dev = input_create_device();
    input_dev->irq_handler = i8042_kbd_handler;

    irq_range_t range = { .first = 4, .last = 16 };
    irq_pick_free_range_size(range, 1, IRQ_SOURCE_IRQ, ANY_CPU, &input_dev->irq);

    port->dev = &input_dev->dev;
    port->dev->drv_data = port;
    port->dev->init_name = "i8042-kbd";
    return device_register(port->dev);
}
