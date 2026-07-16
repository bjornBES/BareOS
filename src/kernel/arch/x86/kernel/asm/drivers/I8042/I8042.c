/*
 * File: I8042.c
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "I8042.h"
#include "I8042_io.h"
#include "I8042_keyboard.h"
#include "kernel/irq.h"
#include "drivers/IO/keyboard/Keyboard.h"
#include "VFS/vfs.h"
#include "debug/debug.h"
#include "kernel/io.h"

#include <util/binary.h>

#include <stdint.h>
#include <stdbool.h>

#define MODULE "I8042"

#define SUPPORT_SECOND_PORT 0

void I8042_keyboard_handler(intr_frame_t *frame, void *ctx)
{
    if (I8042_first_channel_device != I8042_CHANNEL_KEYBOARD_INITALIZED)
    {
        return;
    }
    uint8_t status = inb(I8042_STATUS_PORT);
    // 00011101
    // check if there are data
    if (BIT_GET(status, I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
    {
        return;
    }

    device_t *dev = ctx;
    I8042_first_channel_handler(dev);
}

void I8042_init()
{
    I8042_first_channel_device = I8042_CHANNEL_NO_DEVICE_CONNECTED;
    I8042_second_channel_device = I8042_CHANNEL_NO_DEVICE_CONNECTED;
    I8042_first_channel_present = I8042_DEVICE_NOT_PRESENT;
    I8042_second_channel_present = I8042_DEVICE_NOT_PRESENT;
    I8042_first_channel_buffer_pointer = 0;
    // initialize PS/2 devices

    // step 1: Disable devices
    I8042_write_cmd(0xAD); // disable 1st port
#if SUPPORT_SECOND_PORT
    I8042_write_cmd(0xA7); // disable 2nd port
#endif

    // step 2: Flush The Output Buffer
    I8042_flush_output_buffer(1000);

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
        // irq_arch_register(1, I8042_keyboard_handler, NULL);
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

    if (BIT_GET(inb(I8042_STATUS_PORT), I8042_STAT_OUTPUT_BUFF_STATE))
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
            if (true)
            {
                uint8_t data = I8042_read(I8042_DATA_PORT);
                uint8_t data2 = 0;

                // Some devices send a second ID byte, not always.
                if (I8042_first_channel_wait_for_response())
                {
                    data2 = I8042_read(I8042_DATA_PORT);
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

                log_debug(MODULE, "First channel device ID: %x %x", data, data2);
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
    conf = 0x23;

    I8042_write_controller_conf(conf);

    if (I8042_first_channel_present == I8042_DEVICE_PRESENT)
    {
        log_info(MODULE, "first channel present with conf = %08b", conf);
        device_t *kb = I8042_keyboard_device();
        irq_arch_register(1, I8042_keyboard_handler, kb);
        device_register(kb);
        // initialize_ps2_keyboard();
    }
    if (I8042_second_channel_present == I8042_DEVICE_PRESENT)
    {
        log_info(MODULE, "second channel present");
        // initialize_ps2_keyboard();
    }
}