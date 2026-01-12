/*
 * soft_timer.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#include <stdint.h>
#include <stdbool.h>

#include "Services/soft_timer.h"

void soft_timer_start(soft_timer_t* t, uint32_t ms)
{
    t->start = HAL_GetTick();
    t->duration = ms;
    t->active = true;
}

bool soft_timer_expired(soft_timer_t* t)
{
    if (!t->active) return false;

    if (HAL_GetTick() - t->start >= t->duration) {
        t->active = false;
        return true;
    }
    return false;
}
