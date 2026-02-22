/*
 * encoder.h
 *
 *  Created on: Feb 19, 2026
 *      Author: kiril
 */

#ifndef INC_DRIVERS_ENCODER_H_
#define INC_DRIVERS_ENCODER_H_

#include <stdint.h>

typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_CW,
    ENCODER_CCW
} encoder_direction_t;

void Encoder_Init(void);
encoder_direction_t Encoder_GetDirection(void);

void Encoder_ButtonIRQHandler(void); // Call from EXTI interrupt
uint8_t Encoder_ButtonPressed(void);
uint8_t Encoder_ButtonLongPressed(void);

void Encoder_ButtonTick(void); // Call periodically (e.g. from main loop)

#endif /* INC_DRIVERS_ENCODER_H_ */
