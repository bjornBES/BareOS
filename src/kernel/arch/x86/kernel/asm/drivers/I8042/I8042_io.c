/*
 * File: I8042_io.c
 * File Created: 21 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "I8042_io.h"
#include "debug/debug.h"
#include "kernel/io.h"

#include <util/binary.h>

#define MODULE "I8042-IO"

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
    while (BIT_GET(inb(I8042_STATUS_PORT), I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
    {
        ;
    }
}

void I8042_wait_input_buffer_empty()
{
    while (BIT_GET(inb(I8042_STATUS_PORT), I8042_STAT_INPUT_BUFF_STATE) == I8042_STAT_INPUT_EMPTY)
    {
        ;
    }
}

void I8042_write(uint8_t port, uint8_t data)
{
    // ENTER_FUNC(MODULE, "0x%x, 0x%x", port, data);
    I8042_wait_input_buffer_empty();
    outb(port, data);
}

uint8_t I8042_read(uint8_t port)
{
    // ENTER_FUNC(MODULE, "0x%x", port);
    I8042_wait_output_buffer_fill();
    return inb(port);
}

void I8042_flush_output_buffer(size_t timeout)
{
    // ENTER_FUNC(MODULE, "%u", timeout);
    for (size_t i = 0; i < timeout; i++)
    {
        if (BIT_GET(inb(I8042_STATUS_PORT), I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
        {
            inb(I8042_DATA_PORT);
        }
    }

    if (BIT_GET(inb(I8042_STATUS_PORT), I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
    {
        log_err(MODULE, "Couldn't flush Output buffer");
        return;
    }
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
        if (BIT_GET(inb(I8042_STATUS_PORT), I8042_STAT_INPUT_BUFF_STATE) == I8042_STAT_INPUT_EMPTY)
        {
            break;
        }
    }

    if (BIT_GET(inb(I8042_STATUS_PORT), I8042_STAT_INPUT_BUFF_STATE) == I8042_STAT_INPUT_EMPTY)
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
        if (I8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL))
        {
            continue;
        }
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
    return I8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL);
}

bool I8042_first_channel_wait_for_reset()
{
    if (I8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL))
    {
        log_err(MODULE, "First channel reset response failed");
        return false;
    }
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
            if (I8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL))
            {
                continue;
            }
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
        if (status & 0x10)
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
