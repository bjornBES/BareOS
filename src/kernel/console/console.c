/*
 * File: console.c
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "console/consolet.h"
#include "console/console.h"

#include "drivers/tty/tty.h"

#include "kerrno.h"

#include "init.h"

status_t console_register(console_t *cons)
{
    return KERRNO_SUCCESSES;
}

status_t __init console_init()
{
    n_tty_init();

    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_CONSOLE);
    
    return KERRNO_SUCCESSES;
}
POSTCORE_INITCALL(console_init);


