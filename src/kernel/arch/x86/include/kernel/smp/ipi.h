/*
 * File: ipi.h
 * File Created: 27 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

// Delivers the interrupt specified in the vector field to the target
// processor or processors.
#define DELI_FIXED           0x0

// Same as fixed mode, except that the interrupt is delivered to the
// processor executing at the lowest priority among the set of
// processors specified in the destination field.
#define DELI_LOWEST_PRIORITY 0x1

// Delivers an SMI interrupt to the target processor or processors.
// The vector field must be programmed to 00H for future compatibility.
#define DELI_SMI             0x2

// Delivers an NMI interrupt to the target processor or processors.
// The vector information is ignored.
#define DELI_NMI             0x4

// Delivers an INIT request to the target processor or processors,
// which causes them to perform an INIT. As a result of this IPI
// message, all the target processors perform an INIT. The vector
// field must be programmed to 00H for future compatibility.
#define DELI_INIT            0x5

// Sends a special “start-up” IPI (called a SIPI) to the target
// processor or processors. The vector typically points to a
// start-up routine that is part of the BIOS boot-strap code.
// IPIs sent with this delivery mode are not automatically retried
// if the source APIC is unable to deliver it. It is up to the
// software to determine if the SIPI was not successfully delivered
// and to reissue the SIPI if necessary.
#define DELI_STARTUP         0x6

#define DEST_PHYS            0x0
#define DEST_LOGIC           0x1

// Indicates that this local APIC has completed sending any previous IPIs.
#define DELI_STATUS_IDLE     0x0

// Indicates that this local APIC has not completed sending the last IPI.
#define DELI_STATUS_PENDING  0x1

#define LEVEL_DE_ASSERT      0x0
#define LEVEL_ASSERT         0x1

#define TRIGGER_EDGE         0x0
#define TRIGGER_LEVEL        0x1

// The destination is specified in the destination field.
#define DEST_NO              0x0

// The issuing APIC is the one and only destination of the IPI.
// This destination shorthand allows software to interrupt the
// processor on which it is executing. An APIC implementation
// is free to deliver the self-interrupt message internally or
// to issue the message to the bus and “snoop” it as with any
// other IPI message.
#define DEST_SELF            0x1

// The IPI is sent to all processors in the system including the
// processor sending the IPI. The APIC will broadcast an IPI message
// with the destination field set to 0xF for Pentium and P6 family
// processors and to 0xFF for Pentium 4 and Intel Xeon processors.
#define DEST_ALL             0x2

// The IPI is sent to all processors in a system with the exception
// of the processor sending the IPI. The APIC broadcasts a message
// with the physical destination mode and destination field set to
// 0xF for Pentium and P6 family processors and to 0xFF for Pentium 4
// and Intel Xeon processors. Support for this destination shorthand
// in conjunction with the lowest-priority delivery mode is model
// specific. For Pentium 4 and Intel Xeon processors, when this
// shorthand is used together with lowest priority delivery mode,
// the IPI may be redirected back to the issuing processor.
#define DEST_EXCLUDE_SELF    0x3

#define MAKE_LOW(vector, deli_mode, dest_mode, deli_status, level, trigger_mode, dest_shorthand)                                                      \
    (((dest_shorthand) << 18) | ((trigger_mode) << 15) | ((level) << 14) | ((deli_status) << 12) | ((dest_mode) << 11) | ((deli_mode) << 8) | vector)
