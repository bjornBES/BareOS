/*
 * File: i8042_io.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "i8042.h"

#include "asm/io_arch.h"

#include "debug/debug.h"

#define MODULE "i8042-io"

bool i8042_check_status(uint8_t bit, uint16_t ticks, uint8_t value)
{
    for (uint16_t i = 0; i < ticks; i++)
    {
        if (BIT_GET(inb(I8042_STATUS_REG), bit) == value)
        {
            return true;
        }
    }
    return false;
}

void i8042_wait_output_buffer_fill()
{
    while (BIT_GET(inb(I8042_STATUS_REG), I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
    {
        ;
    }
}

void i8042_wait_input_buffer_empty()
{
    while (BIT_GET(inb(I8042_STATUS_REG), I8042_STAT_INPUT_BUFF_STATE) == I8042_STAT_INPUT_EMPTY)
    {
        ;
    }
}

void i8042_write(uint8_t port, uint8_t data)
{
    ENTER_FUNC("0x%x, 0x%x", port, data);
    i8042_wait_input_buffer_empty();
    outb(port, data);
}

uint8_t i8042_read(uint8_t port)
{
    ENTER_FUNC("0x%x", port);
    i8042_wait_output_buffer_fill();
    return inb(port);
}

void i8042_flush_output_buffer(size_t timeout)
{
    ENTER_FUNC("%u", timeout);
    for (size_t i = 0; i < timeout; i++)
    {
        if (BIT_GET(inb(I8042_STATUS_REG), I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
        {
            inb(I8042_DATA_REG);
        }
    }

    if (BIT_GET(inb(I8042_STATUS_REG), I8042_STAT_OUTPUT_BUFF_STATE) == I8042_STAT_OUTPUT_FULL)
    {
        trace_err(MODULE, "Couldn't flush Output buffer");
        return;
    }
}

void i8042_write_cmd(uint8_t cmd)
{
    i8042_write(I8042_COMMAND_REG, cmd);
}

void i8042_write_controller_conf(uint8_t conf)
{
    i8042_write_cmd(0x60);
    i8042_write(I8042_DATA_REG, conf);
}

uint8_t i8042_read_controller_conf()
{
    i8042_write_cmd(0x20);
    i8042_wait_output_buffer_fill();
    return i8042_read(I8042_DATA_REG);
}

void i8042_write_first_channel(uint8_t data)
{
    uint32_t ticks = 1000;
    for (size_t i = 0; i < ticks; i++)
    {
        if (BIT_GET(inb(I8042_STATUS_REG), I8042_STAT_INPUT_BUFF_STATE) == I8042_STAT_INPUT_EMPTY)
        {
            break;
        }
    }

    if (BIT_GET(inb(I8042_STATUS_REG), I8042_STAT_INPUT_BUFF_STATE) == I8042_STAT_INPUT_EMPTY)
    {
        trace_err(MODULE, "writing to first channel failed due to time-out");
        return;
    }

    i8042_write(I8042_DATA_REG, data);
}

status_t i8042_first_channel_wait_for_ack()
{
    for (uint16_t ticks = 0; ticks < 250; ticks++)
    {
        if (i8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL))
        {
            continue;
        }
        uint8_t data = inb(I8042_DATA_REG);
        if (data == 0xFA)
        {
            return KERRNO_SUCCESSES; // acknowledge sended
        }
    }
    trace_err(MODULE, "Timer ran out in polling first channel");
    return KERRNO_UNSUCCESS;
}

bool i8042_first_channel_wait_for_response()
{
    return i8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL);
}

status_t i8042_first_channel_wait_for_reset()
{
    if (i8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL))
    {
        trace_err(MODULE, "First channel reset response failed");
        return KERRNO_UNSUCCESS;
    }
    uint8_t result = inb(I8042_DATA_REG);
    if (result != 0xAA)
    {
        trace_err(MODULE, "First channel reset self-test failed: 0x%X", result);
        return KERRNO_UNSUCCESS;
    }
    return KERRNO_SUCCESSES;
}

void i8042_write_second_channel(uint8_t data)
{
    i8042_write_cmd(0xD4);
    i8042_write(I8042_DATA_REG, data);
}

status_t i8042_second_channel_wait_for_ack()
{
    uint16_t ticks = 0;
    while (ticks < 250)
    {
        uint8_t status = i8042_read(I8042_STATUS_REG);
        if ((status & 0x10))
        {
            if (i8042_check_status(I8042_STAT_OUTPUT_BUFF_STATE, 1000, I8042_STAT_OUTPUT_FULL))
            {
                continue;
            }
            uint8_t data = i8042_read(I8042_DATA_REG);
            if (data == 0xFA)
            {
                return KERRNO_SUCCESSES; // acknowledge sended
            }
        }
        ticks++;
    }
    trace_err(MODULE, "Timer ran out in polling second channel");
    return KERRNO_UNSUCCESS;
}

bool i8042_second_channel_wait_for_response()
{
    uint16_t ticks = 0;
    while (ticks < 250)
    {
        uint8_t status = i8042_read(I8042_STATUS_REG);
        if (status & 0x10)
        {
            return true;
        }
        ticks++;
    }

    return false;
}
