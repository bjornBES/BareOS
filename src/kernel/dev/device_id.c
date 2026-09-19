/*
 * File: device_id.c
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "dev/device_id.h"
#include "dev/device.h"

#include "string.h"

#define MODULE             "DEVICE"

#define MAX_DEVICE_CLASSES 32

typedef struct
{
    char name[DEVICE_NAME_MAX]; // "hpet", "pit", "lapic", "tty", ...
    uint32_t next_id;
} device_class_counter_t;

static device_class_counter_t class_counters[MAX_DEVICE_CLASSES];
static int class_counter_count = 0;

uint32_t device_id_counters[DEVICE_TYPE_MAX] = {0};

uint32_t device_id_get_id(device_t *dev)
{
    uint32_t result = device_id_counters[dev->class];
    device_id_counters[dev->class]++;
    return result;
}

uint32_t device_id_get_next_class_id(const char *class_name)
{
    for (int i = 0; i < class_counter_count; i++)
    {
        if (strcmp(class_counters[i].name, class_name) == 0)
        {
            return class_counters[i].next_id++;
        }
    }

    // new class name, start at 0
    strncpy(class_counters[class_counter_count].name, class_name, DEVICE_NAME_MAX - 1);
    class_counters[class_counter_count].next_id = 1; // returning 0 this call
    class_counter_count++;
    return 0;
}
