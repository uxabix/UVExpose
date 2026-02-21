/*
 * power_channel.c
 * Hardware abstraction layer for UV LED power control
 *
 *  Created on: Feb 21, 2026
 *      Author: kiril
 */

#include "App/power_channel.h"
#include "gpio.h"
#include "main.h"
#include "config.h"

static uint8_t channel_enabled = 0;

void PowerChannel_Init(void)
{
    // PIN already initialized in MX_GPIO_Init()
    // Just ensure it's off on startup
    PowerChannel_Disable();
}

void PowerChannel_Enable(void)
{
    HAL_GPIO_WritePin(LED_Output_GPIO_Port, LED_Output_Pin, LED_OUTPUT_ACTIVE_LEVEL);
    channel_enabled = 1;
}

void PowerChannel_Disable(void)
{
    // Set to the inverse of active level
    // If LED_OUTPUT_ACTIVE_LEVEL is GPIO_PIN_SET, then disable = GPIO_PIN_RESET
    // If LED_OUTPUT_ACTIVE_LEVEL is GPIO_PIN_RESET, then disable = GPIO_PIN_SET
    uint8_t disable_level = (LED_OUTPUT_ACTIVE_LEVEL == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(LED_Output_GPIO_Port, LED_Output_Pin, disable_level);
    channel_enabled = 0;
}

uint8_t PowerChannel_IsEnabled(void)
{
    return channel_enabled;
}
