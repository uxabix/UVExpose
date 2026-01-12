/*
 * soft_timer.h
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#ifndef INC_SOFT_TIMER_H_
#define INC_SOFT_TIMER_H_

typedef struct {
    uint32_t start;
    uint32_t duration;
    bool active;
} soft_timer_t;

void soft_timer_start(soft_timer_t* t, uint32_t ms);
bool soft_timer_expired(soft_timer_t* t);


#endif /* INC_SOFT_TIMER_H_ */
