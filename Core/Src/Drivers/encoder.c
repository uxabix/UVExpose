/*
 * encoder.c
 *
 *  Created on: Feb 19, 2026
 *      Author: kiril
 */

#include <gpio.h>
#include <tim.h>

#include "Drivers/encoder.h"


#define BUTTON_DEBOUNCE_MS 20
#define BUTTON_LONG_MS     800

static uint8_t short_press = 0;
static uint8_t long_press = 0;


void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}

encoder_direction_t Encoder_GetDirection(void)
{
    static int16_t last = 0;
    int16_t current = __HAL_TIM_GET_COUNTER(&htim2);

    if(current > last)
    {
        last = current;
        return ENCODER_CW;
    }
    else if(current < last)
    {
        last = current;
        return ENCODER_CCW;
    }

    return ENCODER_NONE;
}

void Encoder_ButtonTask(void)
{
    static uint8_t stable_state = 1;     // 1 = released (pull-up)
    static uint8_t last_raw = 1;
    static uint32_t last_change_time = 0;

    static uint32_t press_start_time = 0;
    static uint8_t long_sent = 0;

    uint8_t raw = HAL_GPIO_ReadPin(ENC_BTN_GPIO_Port, ENC_BTN_Pin);
    uint32_t now = HAL_GetTick();

    // Debounce
    if(raw != last_raw)
    {
        last_change_time = now;
        last_raw = raw;
    }

    if((now - last_change_time) >= BUTTON_DEBOUNCE_MS)
    {
        if(stable_state != raw)
        {
            stable_state = raw;

            if(stable_state == 0)
            {
                // Button pressed
                press_start_time = now;
                long_sent = 0;
            }
            else
            {
                // Button released
                uint32_t held = now - press_start_time;

                if(held >= BUTTON_DEBOUNCE_MS && held < BUTTON_LONG_MS)
                    short_press = 1;
            }
        }
    }

    // Hold
    if(stable_state == 0 && !long_sent)
    {
        if((now - press_start_time) >= BUTTON_LONG_MS)
        {
            long_press = 1;
            long_sent = 1;
        }
    }
}



uint8_t Encoder_ButtonPressed(void){
	if (short_press) {
		short_press = 0;
		return 1;
	}
	return 0;
}

uint8_t Encoder_ButtonLongPressed(void){
	if (long_press) {
		long_press = 0;
		return 1;
	}
	return 0;
}
