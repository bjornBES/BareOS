/*
 * File: UART.c
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

/*
 * File: UART.c
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "UART.h"
#include "printf_driver/printf.h"

#include "libs/IO.h"

int UART_serial_received(uint16_t port) {
   return inb(port + UART_LINE_STATUS_REGISTER) & 1;
}

int UART_is_transmit_empty(uint16_t port) {
   return inb(port + UART_LINE_STATUS_REGISTER) & 0x20;
}

uint8_t UART_read(uint16_t port, UART_port off)
{
   while (UART_serial_received(port) == 0);

   return inb(port + off);
}

void UART_write(uint16_t port, UART_port off, uint8_t data)
{
   while (UART_is_transmit_empty(port) == 0);

   outb(port + off, data);
}

void UART_write_char(uint16_t port, uint8_t data)
{
    UART_write(port, UART_DATA, data);
}
void UART_write_str(uint16_t port, char* data)
{
    while (*data)
    {
        UART_write(port, UART_DATA, *data);
        data++;
    }
}
void UART_write_fstr(uint16_t port, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char str[1024];
    vsprintf(str, format, args);
    UART_write_str(port, str);
    va_end(args);
}

bool UART_init(uint16_t port)
{
    UART_write(port, UART_INTERRUPT_ENABLE, 0x00);
    UART_write(port, UART_LINE_CONTROL_REGISTER, 0x80);
    UART_write(port, UART_DLAB_LSB, 0x03);
    UART_write(port, UART_DLAB_MSB, 0x00);
    UART_write(port, UART_LINE_CONTROL_REGISTER, 0x03);
    UART_write(port, UART_FIFO_CONTROL_REGISTER, 0xC7);
    UART_write(port, UART_MODEM_CONTROL_REGISTER, 0x0B);
    UART_write(port, UART_MODEM_CONTROL_REGISTER, 0x1E);
    UART_write(port, UART_DATA, 0xAE);
    
    if (UART_read(port, UART_DATA) != 0xAE)
    {
        return false;
    }
    
    UART_write(port, UART_MODEM_CONTROL_REGISTER, 0x0F);
    return true;
}