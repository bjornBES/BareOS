/*
 * File: semaphore.h
 * File Created: 08 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "semaphoret.h"

#include <types.h>

void sem_init(semaphore_t *s, int initial);
void sem_wait(semaphore_t *s);
void sem_post(semaphore_t *s);