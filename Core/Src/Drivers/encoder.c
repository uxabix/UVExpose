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
static uint8_t button_state = 1; // 1 = released, 0 = pressed
static uint32_t press_start_time = 0;
static uint8_t long_sent = 0;


void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}

encoder_direction_t Encoder_GetDirection(void)
{
    static int16_t last = 0;
    static int8_t accumulator = 0;

    int16_t current = __HAL_TIM_GET_COUNTER(&htim2);
    int16_t diff = current - last;
    last = current;

    accumulator += diff;

    if(accumulator >= 4)
    {
        accumulator = 0;
        return ENCODER_CW;
    }
    else if(accumulator <= -4)
    {
        accumulator = 0;
        return ENCODER_CCW;
    }

    return ENCODER_NONE;
}

void Encoder_ButtonIRQHandler(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t state = HAL_GPIO_ReadPin(ENC_BTN_GPIO_Port, ENC_BTN_Pin);

    if(state == 0 && button_state == 1) // Button pressed (falling edge)
    {
        press_start_time = now;
        long_sent = 0;
        button_state = 0;
    }
    else if(state == 1 && button_state == 0) // Button released (rising edge)
    {
        uint32_t held = now - press_start_time;
        if(held >= BUTTON_DEBOUNCE_MS && held < BUTTON_LONG_MS && !long_sent)
            short_press = 1;
        button_state = 1;
    }
}

void Encoder_ButtonTick(void)
{
    if(button_state == 0 && !long_sent)
    {
        uint32_t now = HAL_GetTick();
        uint32_t held = now - press_start_time;
        if(held >= BUTTON_LONG_MS)
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
