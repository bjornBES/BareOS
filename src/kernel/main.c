
#include <stdio.h>
#include <stdint.h>

#include "hal/hal.h"

void crash_me();

void main()
{
    printf("Hello world from Kernel\n");

    HALInit();

    crash_me();

    // loop
    for (;;)
        ;
}