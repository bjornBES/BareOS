/*
 * File: I8042.c
 * File Created: 12 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "I8042.h"
#include "arch/i686/irq.h"
#include "arch/i686/i8259.h"
#include "drivers/IO/Keyboard/Keyboard.h"

#include <util/binary.h>
#include <debug/debug.h>
#include <IO.h>

#include <stdint.h>
#include <stdbool.h>

#define MODULE "I8042"

#define SUPPORT_SECOND_PORT 0

// in bits

#define I8042_OUTPUT_BUFFER_STATE 0
#define I8042_INPUT_BUFFER_STATE 1

uint8_t I8042_first_channel_present;
uint8_t I8042_second_channel_present;
uint8_t I8042_first_channel_device;
uint8_t I8042_second_channel_device;

uint8_t I8042_first_channel_buffer[10];
uint8_t I8042_second_channel_buffer[10];
uint8_t I8042_first_channel_buffer_pointer;
uint8_t I8042_second_channel_buffer_pointer;

bool I8042_check_status(uint8_t bit, uint16_t ticks, uint8_t value)
{
    for (uint16_t i = 0; i < ticks; i++)
    {
        if (BIT_GET(inb(I8042_STATUS_PORT), bit) == value)
        {
            return true;
        }
    }
    return false;
}
void I8042_wait_output_buffer_fill()
{
    while (!BIT_GET(inb(I8042_STATUS_PORT), I8042_OUTPUT_BUFFER_STATE))
    {
        ;
    }
}
void I8042_wait_input_buffer_empty()
{
    while (BIT_GET(inb(I8042_STATUS_PORT), I8042_INPUT_BUFFER_STATE))
    {
        ;
    }
}

void I8042_write(uint8_t port, uint8_t data)
{
    I8042_wait_input_buffer_empty();
    outb(port, data);
}
uint8_t I8042_read(uint8_t port)
{
    I8042_wait_output_buffer_fill();
    return inb(port);
}

void I8042_write_cmd(uint8_t cmd)
{
    I8042_write(I8042_COMMAND_PORT, cmd);
}
void I8042_write_controller_conf(uint8_t conf)
{
    I8042_write_cmd(0x60);
    I8042_write(I8042_DATA_PORT, conf);
}
uint8_t I8042_read_controller_conf()
{
    I8042_write_cmd(0x20);
    I8042_wait_output_buffer_fill();
    return I8042_read(I8042_DATA_PORT);
}

void I8042_write_first_channel(uint8_t data)
{
    uint32_t ticks = 1000;
    for (size_t i = 0; i < ticks; i++)
    {
        if (!BIT_GET(inb(I8042_STATUS_PORT), I8042_INPUT_BUFFER_STATE))
        {
            break;
        }
    }

    if (BIT_GET(inb(I8042_STATUS_PORT), I8042_INPUT_BUFFER_STATE))
    {
        log_err(MODULE, "writing to first channel failed due to time-out");
        return;
    }

    I8042_first_channel_buffer_pointer = 0;
    I8042_first_channel_buffer[0] = 0;
    I8042_write(I8042_DATA_PORT, data);
}

bool I8042_first_channel_wait_for_ack()
{
    for (uint16_t ticks = 0; ticks < 250; ticks++)
    {
        if (!I8042_check_status(I8042_OUTPUT_BUFFER_STATE, 1000, 1))
            continue;
        uint8_t data = inb(I8042_DATA_PORT);
        if (data == 0xFA)
        {
            return true; // acknowledge sended
        }
    }
    log_err(MODULE, "Timer ran out in polling first channel");
    return false;
}

bool I8042_first_channel_wait_for_response()
{
    uint16_t ticks = 0;
    while (ticks < 250)
    {
        uint8_t status = inb(I8042_STATUS_PORT);
        if (BIT_GET(status, I8042_OUTPUT_BUFFER_STATE))
        {
            return true;
        }
        ticks++;
    }

    return false;
}
bool I8042_first_channel_wait_for_reset()
{
    if (!I8042_first_channel_wait_for_response())
        return false;
    uint8_t result = inb(I8042_DATA_PORT);
    if (result != 0xAA)
    {
        log_err(MODULE, "First channel reset self-test failed: 0x%X", result);
        return false;
    }
    return true;
}

void I8042_write_second_channel(uint8_t data)
{
    I8042_second_channel_buffer_pointer = 0;
    I8042_second_channel_buffer[0] = 0;
    I8042_write_cmd(0xD4);
    I8042_write(I8042_DATA_PORT, data);
}

bool I8042_second_channel_wait_for_ack()
{
    uint16_t ticks = 0;
    while (ticks < 250)
    {
        uint8_t status = I8042_read(I8042_STATUS_PORT);
        if ((status & 0x10) || I8042_second_channel_buffer_pointer > 0)
        {
            if (!I8042_check_status(I8042_OUTPUT_BUFFER_STATE, 1000, 1))
                continue;
            uint8_t data = I8042_read(I8042_DATA_PORT);
            if (data == 0xFA || I8042_second_channel_buffer[0] == 0xFA)
            {
                return true; // acknowledge sended
            }
        }
        ticks++;
    }
    log_err(MODULE, "Timer ran out in polling second channel");
    return false;
}

bool I8042_second_channel_wait_for_response()
{
    uint16_t ticks = 0;
    while (ticks < 250)
    {
        uint8_t status = I8042_read(I8042_STATUS_PORT);
        if ((status & 0x10))
        {
            return true;
        }
        if (I8042_second_channel_buffer_pointer > 1)
        {
            return true;
        }
        ticks++;
    }

    return false;
}

void I8042_first_channel_handler(Registers *regs)
{
    uint8_t status = I8042_read(I8042_STATUS_PORT);
    // check if there are data
    if ((status & 0x1) != 0x1)
    {
        return;
    }

    // read data
    I8042_first_channel_buffer[I8042_first_channel_buffer_pointer] = I8042_read(I8042_DATA_PORT);

    log_debug(MODULE, "I8042_first_channel_handler: data = 0x%X", I8042_first_channel_buffer[I8042_first_channel_buffer_pointer]);

    // move pointer
    I8042_first_channel_buffer_pointer++;
    if (I8042_first_channel_buffer_pointer >= 10)
    {
        log_err(MODULE, "first channel buffer is full");
        I8042_first_channel_buffer_pointer = 0;
    }

    // process data
    if (I8042_first_channel_device == I8042_CHANNEL_KEYBOARD_INITALIZED)
    {
        uint8_t pointer = I8042_first_channel_buffer_pointer;
        uint8_t raw = I8042_first_channel_buffer[pointer - 1];
        key_event ev;
        if (keyboard_process_byte(raw, &ev))
        {
            keyboard_put_key(&ev);
            log_debug(MODULE, "Key %s: %d", ev.action == KEY_PRESSED ? "press" : "release", ev.key);
        }

        // clear variables
        I8042_first_channel_buffer_pointer = 0;
    }
    i8259_send_eoi(1);
}

void I8042_init()
{
    I8042_first_channel_device = I8042_CHANNEL_NO_DEVICE_CONNECTED;
    I8042_second_channel_device = I8042_CHANNEL_NO_DEVICE_CONNECTED;
    I8042_first_channel_present = I8042_DEVICE_NOT_PRESENT;
    I8042_second_channel_present = I8042_DEVICE_NOT_PRESENT;
    I8042_first_channel_buffer_pointer = 0;
    // Initialize PS/2 devices

    // step 1: Disable devices
    I8042_write_cmd(0xAD); // disable 1st port
#if SUPPORT_SECOND_PORT
    I8042_write_cmd(0xA7); // disable 2nd port
#endif

    // step 2: Flush The Output Buffer
    for (uint16_t i = 0; i < 1000; i++)
    {
        if (BIT_GET(inb(I8042_STATUS_PORT), I8042_OUTPUT_BUFFER_STATE))
        {
            inb(I8042_DATA_PORT);
        }
    }

    if (BIT_GET(inb(I8042_STATUS_PORT), I8042_OUTPUT_BUFFER_STATE))
    {
        log_err(MODULE, "Couldn't flush Output buffer");
        return;
    }

    // step 3: Set the Controller Configuration Byte
    uint8_t conf = I8042_read_controller_conf();
    BIT_UNSET(conf, 0); // Disable first PS/2 port interrupts
    BIT_UNSET(conf, 6); // Disable first PS/2 port translation
    BIT_UNSET(conf, 4); // Enable first PS/2 port clock

    BIT_UNSET(conf, 1); // Disable second PS/2 port interrupts
    I8042_write_controller_conf(conf);

    // step 4: Perform Controller Self Test
    I8042_write_cmd(0xAA);
    if (I8042_read(I8042_DATA_PORT) != 0x55)
    {
        log_err(MODULE, "self test failed");
        return;
    }

    // step 5: Determine If There Are 2 Channels
    I8042_first_channel_present = I8042_DEVICE_PRESENT;

#if SUPPORT_SECOND_PORT
    I8042_write_cmd(0xA8);

    conf = I8042_read_controller_conf();

    I8042_second_channel_present = I8042_DEVICE_NOT_PRESENT;
    if (!BIT_GET(conf, 5))
    {
        I8042_second_channel_present = I8042_DEVICE_PRESENT;
    }
    
    I8042_write_cmd(0xAD); // disable 1st port
    if (I8042_second_channel_present == I8042_DEVICE_PRESENT)
    {
        BIT_UNSET(conf, 1);    // disable interrupts
        BIT_UNSET(conf, 5);    // enable clock port 2
        I8042_write_cmd(0xA7); // disable 2nd port
        I8042_write_controller_conf(conf);
    }
#endif
    
    // step 6: Perform Interface Tests + Enable Interrupts
    if (I8042_first_channel_present == I8042_DEVICE_PRESENT)
    {
        I8042_write_cmd(0xAB);
        if (I8042_read(I8042_DATA_PORT) != 0)
        {
            log_err(MODULE, "failed interface test for first channel");
            I8042_first_channel_present = I8042_DEVICE_PRESENT_BUT_ERROR_STATE;
        }
    }
#if SUPPORT_SECOND_PORT
    if (I8042_second_channel_present == I8042_DEVICE_PRESENT)
    {
        I8042_write_cmd(0xA9);
        if (I8042_read(I8042_DATA_PORT) != 0)
        {
            log_err(MODULE, "failed interface test for second channel");
            I8042_second_channel_present = I8042_DEVICE_PRESENT_BUT_ERROR_STATE;
        }
    }
#endif

    conf = I8042_read_controller_conf();
    if (I8042_first_channel_present == I8042_DEVICE_PRESENT)
    {
        BIT_UNSET(conf, 0); // disable interrupts
        i686_irq_register_handler(1, I8042_first_channel_handler);
        // handlers
    }
#if SUPPORT_SECOND_PORT
    if (I8042_second_channel_present == I8042_DEVICE_PRESENT)
    {
        // handlers
        BIT_UNSET(conf, 1); // disable interrupts
    }
#endif
    I8042_write_controller_conf(conf);

    // step 7: Enable Devices
    if (I8042_first_channel_present == I8042_DEVICE_PRESENT)
    {
        I8042_write_cmd(0xAE);
        I8042_write_first_channel(0xFF); // reset
        if (I8042_first_channel_wait_for_ack() == false)
        {
            log_err(MODULE, "First channel no ACK after reset");
        }
        if (I8042_first_channel_wait_for_reset() == false)
        {
            log_err(MODULE, "First channel no response after reset");
        }
    }
#if SUPPORT_SECOND_PORT
    if (I8042_second_channel_present == I8042_DEVICE_PRESENT)
    {
        I8042_write_cmd(0xA8);
        I8042_write_second_channel(0xFF); // reset
        if (I8042_second_channel_wait_for_ack() == false)
        {
            log_err(MODULE, "Second channel no ACK after reset");
        }
        if (I8042_second_channel_wait_for_response() == false)
        {
            log_err(MODULE, "Second channel no response after reset");
        }
    }
#endif

    conf = I8042_read_controller_conf();

    if (BIT_GET(inb(I8042_STATUS_PORT), I8042_OUTPUT_BUFFER_STATE))
    {
        log_err(MODULE, "Couldn't flush Output buffer");
        return;
    }
    // check what type of device is connected on first channel
    if (I8042_first_channel_present == I8042_DEVICE_PRESENT)
    {
        // after reset device should not be in streaming mode, but to be sure disable streaming
        I8042_write_first_channel(0xF5);
        if (I8042_first_channel_wait_for_ack() == false)
        {
            log_err(MODULE, "First channel no ACK after 0xF5");
        }

        // read device ID
        I8042_write_first_channel(0xF2);
        if (I8042_first_channel_wait_for_ack() == true)
        {
            if (I8042_first_channel_wait_for_response() == true)
            {
                uint8_t data = I8042_read(I8042_DATA_PORT);

                // Some devices send a second ID byte, not always.
                if (I8042_first_channel_wait_for_response())
                {
                    I8042_read(I8042_DATA_PORT);
                }

                if (data == 0xAA || data == 0xAB || data == 0xAC)
                {
                    I8042_first_channel_device = I8042_CHANNEL_KEYBOARD_CONNECTED;
                }
                else
                {
                    I8042_first_channel_device = data; // unknown device
                }

                I8042_first_channel_buffer_pointer = 0;
                I8042_first_channel_buffer[0] = 0;
                I8042_write_first_channel(0xF4);
                if (I8042_first_channel_wait_for_ack() == true)
                {
                    I8042_first_channel_buffer_pointer = 0;
                    I8042_first_channel_device = I8042_CHANNEL_KEYBOARD_INITALIZED;
                }
                else
                {
                    log_err(MODULE, "First channel keyboard not ACKed");
                }

                // enable interrupts
                if (I8042_first_channel_present == I8042_DEVICE_PRESENT)
                {
                    conf |= (1 << 0); // enable interrupts
                }

                log_debug(MODULE, "First channel device ID: %x", data);
            }
            else
            {
                log_err(MODULE, "First channel device did not send ID data");
            }
        }
        else
        {
            log_err(MODULE, "First channel device did not send ID ack");
        }
    }

#if SUPPORT_SECOND_PORT
    // check what type of device is connected on second channel
    if (I8042_second_channel_present == I8042_DEVICE_PRESENT)
    {
        // after reset device should not be in streaming mode, but to be sure disable streaming
        I8042_write_second_channel(0xF5);
        if (I8042_second_channel_wait_for_ack() == false)
        {
            log_err(MODULE, "ERROR: Second channel no ACK after 0xF5");
        }

        // read device ID
        I8042_second_channel_buffer[1] = 0xFF;
        I8042_write_second_channel(0xF2);
        if (I8042_second_channel_wait_for_ack() == true)
        {
            if (I8042_second_channel_wait_for_response() == true)
            {
                if (I8042_second_channel_buffer[1] == 0x00 || I8042_second_channel_buffer[1] == 0x03 || I8042_second_channel_buffer[1] == 0x04)
                {
                    I8042_second_channel_device = I8042_CHANNEL_MOUSE_CONNECTED;
                }
                else
                {
                    I8042_second_channel_device = I8042_second_channel_buffer[1]; // unknown device
                }

                I8042_write_second_channel(0xF4);
                I8042_second_channel_buffer[0] = 0;
                I8042_second_channel_buffer_pointer = 0;
                if (I8042_second_channel_wait_for_ack() == true)
                {
                    I8042_second_channel_buffer_pointer = 0;
                    I8042_second_channel_device = I8042_CHANNEL_KEYBOARD_INITALIZED;
                }
            }
            else
            {
                log_err(MODULE, "ERROR: Second channel device did not send ID data");
            }
        }
        else
        {
            log_err(MODULE, "ERROR: Second channel device did not send ID ack");
        }
    }
#endif
    I8042_write_controller_conf(conf);

    if (I8042_first_channel_present == I8042_DEVICE_PRESENT)
    {
        log_info(MODULE, "first channel present with conf = %08b", conf);
        keyboard_init();
        // initialize_ps2_keyboard();
    }
    if (I8042_second_channel_present == I8042_DEVICE_PRESENT)
    {
        log_info(MODULE, "second channel present");
        // initialize_ps2_keyboard();
    }
}