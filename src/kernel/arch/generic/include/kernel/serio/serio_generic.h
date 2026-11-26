/*
 * File: serio_generic.h
 * File Created: 26 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

/*
 * Serio types
 */
#define SERIO_XT	0x00
#define SERIO_8042	0x01
#define SERIO_RS232	0x02
#define SERIO_HIL_MLC	0x03
#define SERIO_PS_PSTHRU	0x05
#define SERIO_8042_XL	0x06


#ifndef I8042_KBD_IRQ
#define I8042_KBD_IRQ -1
#endif