/*
 * File: UART.h
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdint.h>
#include <stdbool.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x3E8
#define COM5 0x5F8
#define COM6 0x4F8
#define COM7 0x5E8
#define COM8 0x4E8

typedef enum
{
    UART_DATA = 0,
    UART_INTERRUPT_ENABLE = 1,
    UART_DLAB_LSB = 0,
    UART_DLAB_MSB = 1,
    UART_INTERRUPT_IDENTIFICATION_REGISTER = 2,
    UART_FIFO_CONTROL_REGISTER = 2,
    UART_LINE_CONTROL_REGISTER = 3,
    UART_MODEM_CONTROL_REGISTER = 4,
    UART_LINE_STATUS_REGISTER = 5,
    UART_MODEM_STATUS_REGISTER = 6,
    UART_SCRATCH_REGISTER = 7,
} UART_port;



uint8_t UART_read(uint16_t port, UART_port off);
void UART_write(uint16_t port, UART_port off, uint8_t data);
bool UART_init(uint16_t port);
void UART_write_char(uint16_t port, uint8_t data);
void UART_write_str(uint16_t port, char* data);
void UART_write_fstr(uint16_t port, const char* format, ...);
