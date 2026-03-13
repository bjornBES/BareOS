/*
 * File: i8259.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

/*
 * File: i8259.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "i8259.h"

#include <IO.h>
#include <stdbool.h>

#define PIC1_COMMAND_PORT           0x20
#define PIC1_DATA_PORT              0x21
#define PIC2_COMMAND_PORT           0xA0
#define PIC2_DATA_PORT              0xA1
#define UNUSED_PORT                 0x80

// Initialization Control Word 1
// -----------------------------
//  0   IC4     if set, the PIC expects to receive ICW4 during initialization
//  1   SGNL    if set, only 1 PIC in the system; if unset, the PIC is cascaded with slave PICs
//              and ICW3 must be sent to controller
//  2   ADI     call address interval, set: 4, not set: 8; ignored on x86, set to 0
//  3   LTIM    if set, operate in level triggered mode; if unset, operate in edge triggered mode
//  4   INIT    set to 1 to initialize PIC
//  5-7         ignored on x86, set to 0

enum {
    PIC_ICW1_ICW4           = 0x01,
    PIC_ICW1_SINGLE         = 0x02,
    PIC_ICW1_INTERVAL4      = 0x04,
    PIC_ICW1_LEVEL          = 0x08,
    PIC_ICW1_INITIALIZE     = 0x10
} PIC_ICW1;


// Initialization Control Word 4
// -----------------------------
//  0   uPM     if set, PIC is in 80x86 mode; if cleared, in MCS-80/85 mode
//  1   AEOI    if set, on last interrupt acknowledge pulse, controller automatically performs 
//              end of interrupt operation
//  2   M/S     only use if BUF is set; if set, selects buffer master; otherwise, selects buffer slave
//  3   BUF     if set, controller operates in buffered mode
//  4   SFNM    specially fully nested mode; used in systems with large number of cascaded controllers
//  5-7         reserved, set to 0
enum {
    PIC_ICW4_8086           = 0x1,
    PIC_ICW4_AUTO_EOI       = 0x2,
    PIC_ICW4_BUFFER_MASTER  = 0x4,
    PIC_ICW4_BUFFER_SLAVE   = 0x0,
    PIC_ICW4_BUFFERRED      = 0x8,
    PIC_ICW4_SFNM           = 0x10,
} PIC_ICW4;


enum {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B,
} PIC_CMD;


static uint16_t g_pic_mask = 0xffff;

void i686_io_wait()
{
    outb(UNUSED_PORT, 0);
}

void i8259_set_mask(uint16_t new_mask)
{
    g_pic_mask = new_mask;
    outb(PIC1_DATA_PORT, g_pic_mask & 0xFF);
    i686_io_wait();
    outb(PIC2_DATA_PORT, g_pic_mask >> 8);
    i686_io_wait();
}

uint16_t i8259_get_mask()
{
    return inb(PIC1_DATA_PORT) | (inb(PIC2_DATA_PORT) << 8);
}

void i8259_configure(uint8_t offset_pic_1, uint8_t offset_pic_2, bool auto_eoi)
{
    // initialization control word 1
    outb(PIC1_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    i686_io_wait();
    outb(PIC2_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    i686_io_wait();
    
    // initialization control word 2 - the offset
    outb(PIC1_DATA_PORT, offset_pic_1);
    i686_io_wait();
    outb(PIC2_DATA_PORT, offset_pic_2);
    i686_io_wait();
    
    // initializaion control word 3
    outb(PIC1_DATA_PORT, 0x4);
    i686_io_wait();
    outb(PIC2_DATA_PORT, 0x2);
    i686_io_wait();
    
    // initializaion control word 4
    uint8_t icw4 = PIC_ICW4_8086;
    if (auto_eoi)
    {
        icw4 |= PIC_ICW4_AUTO_EOI;
    }

    outb(PIC1_DATA_PORT, icw4);
    i686_io_wait();
    outb(PIC2_DATA_PORT, icw4);
    i686_io_wait();

    // mask all interrupts
    // until they are enabled by the device driver
    i8259_set_mask(0xFFFF);
}

void i8259_send_eoi(int irq)
{
    if (irq >= 8)
    {
        outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
    }
    outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
}

void i8259_disable()
{
    i8259_set_mask(0xFFFF);
}

void i8259_mask(int irq)
{
    i8259_set_mask(g_pic_mask | (1 << irq));
}

void i8259_unmask(int irq)
{
    i8259_set_mask(g_pic_mask & ~(1 << irq));
}

uint16_t i8259_read_irq_request_register()
{
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    return ((uint16_t)inb(PIC2_COMMAND_PORT)) | (((uint16_t)inb(PIC2_COMMAND_PORT)) << 8);
}

uint16_t i8259_read_in_service_register()
{
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    return ((uint16_t)inb(PIC2_COMMAND_PORT)) | (((uint16_t)inb(PIC2_COMMAND_PORT)) << 8);
}


void sys_sleep(unsigned long seconds)
{
    i8259_mask(1);
    
}

bool i8259_probe()
{
    i8259_disable();
    i8259_set_mask(0x1337);
    return i8259_get_mask() == 0x1337;
}

static const PICDriver g_pic_driver = {
    .Name = "8259 PIC",
    .Probe = &i8259_probe,
    .Initialize = &i8259_configure,
    .Disable = &i8259_disable,
    .SendEndOfInterrupt = &i8259_send_eoi,
    .Mask = &i8259_mask,
    .Unmask = &i8259_unmask,
};

const PICDriver* i8259_get_driver()
{
    return &g_pic_driver;  
}
