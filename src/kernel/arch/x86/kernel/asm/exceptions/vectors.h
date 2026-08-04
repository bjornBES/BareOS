/*
 * File: vectors.h
 * File Created: 29 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define EXC_DB                   0x01 // Debug
#define EXC_BP                   0x03 // Breakpoint
#define EXC_UD                   0x06 // Invalid Opcode
#define EXC_DF                   0x08 // Double Fault
#define EXC_NP                   0x0B // Segment Not Present
#define EXC_GP                   0x0D // General Protection Fault
#define EXC_PF                   0x0E // Page Fault

#define IRQ0                     0x20
#define IRQ23                    0x37

#define IPI_START                0x40
#define IPI_CALL_FUNCTION_VECTOR 0x40
#define IPI_CPUS_IDLE_VECTOR     0x41
#define IPI_END_PRINTABLE        0x50
#define IPI_RESCHEDULE_VECTOR    0x51
#define IPI_END                  0x60

#define SCHED_SCHEDULE           0x7F
#define EXC_SYSCALL              0x80
#define LAPIC_TIMER_VECTOR       0xA0

#define PIC_MSI_VEC1             0xE0
#define PIC_MSI_VEC2             0xE1
#define PIC_MSI_VEC3             0xE2
#define PIC_MSI_VEC4             0xE3

// IRQs

#define IRQ_KB 1 // keyboard

