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
static uint8_t button_state = 1; // debounced state: 1 = released, 0 = pressed
static uint8_t sampled_state = 1; // raw sampled state for debounce
static uint32_t press_start_time = 0;
static uint32_t last_state_change_time = 0;
static uint8_t long_sent = 0;

static void Encoder_ButtonProcessSample(uint8_t raw_state, uint32_t now)
{
    if (raw_state != sampled_state) {
        sampled_state = raw_state;
        last_state_change_time = now;
    }

    if ((now - last_state_change_time) >= BUTTON_DEBOUNCE_MS) {
        if (button_state != sampled_state) {
            button_state = sampled_state;

            if (button_state == 0u) {
                // Stable press detected.
                press_start_time = now;
                long_sent = 0u;
            } else {
                // Stable release detected.
                uint32_t held = now - press_start_time;
                if (held < BUTTON_LONG_MS && !long_sent) {
                    short_press = 1u;
                }
            }
        }
    }

    if (button_state == 0u && !long_sent) {
        uint32_t held = now - press_start_time;
        if (held >= BUTTON_LONG_MS) {
            long_press = 1u;
            long_sent = 1u;
        }
    }
}

void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    uint8_t raw_state = HAL_GPIO_ReadPin(ENC_BTN_GPIO_Port, ENC_BTN_Pin);
    button_state = raw_state;
    sampled_state = raw_state;
    last_state_change_time = HAL_GetTick();
    press_start_time = last_state_change_time;
    long_sent = 0u;
    short_press = 0u;
    long_press = 0u;
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
    // Button state is processed in polling mode from Encoder_ButtonTick().
    // Keeping IRQ hook as a no-op avoids edge-burst jitter affecting debounce.
}

void Encoder_ButtonTick(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t raw_state = HAL_GPIO_ReadPin(ENC_BTN_GPIO_Port, ENC_BTN_Pin);
    Encoder_ButtonProcessSample(raw_state, now);
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
