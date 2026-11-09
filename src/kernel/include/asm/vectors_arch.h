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

#define EXC_DEBUG                0x01 // Debug
#define EXC_BREAKPOINT           0x03 // Breakpoint
#define EXC_INVALID_OPCODE       0x06 // Invalid Opcode
#define EXC_DF                   0x08 // Double Fault
#define EXC_NP                   0x0B // Segment Not Present
#define EXC_GP                   0x0D // General Protection Fault
#define EXC_FAULT                0x0E // Page Fault

#define EXC_END                  0x20
#define IRQ0                     0x20
#define IRQ23                    0x37

#define EXC_SYSCALL              0x80

#define MAX_IRQ_VECTORS          0xCF
#define KERNEL_VECTOR_START      0xD0
#define SCHED_SCHEDULE           0xD1
#define IPI_START                0xD2
#define IPI_CALL_FUNCTION_VECTOR 0xD2
#define IPI_CPUS_IDLE_VECTOR     0xD3
#define IPI_END_PRINTABLE        0xE0
#define IPI_RESCHEDULE_VECTOR    0xE1
#define IPI_END                  0xEE
#define CPU_TIMER_VECTOR         0xEF

#define MAX_VECTOR               255
