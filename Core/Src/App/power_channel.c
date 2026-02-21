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

static uint8_t channel_enabled = 0;

void PowerChannel_Init(void)
{
    // PIN already initialized in MX_GPIO_Init()
    // Just ensure it's off on startup
    PowerChannel_Disable();
}

void PowerChannel_Enable(void)
{
    HAL_GPIO_WritePin(LED_Output_GPIO_Port, LED_Output_Pin, GPIO_PIN_SET);
    channel_enabled = 1;
}

void PowerChannel_Disable(void)
{
    HAL_GPIO_WritePin(LED_Output_GPIO_Port, LED_Output_Pin, GPIO_PIN_RESET);
    channel_enabled = 0;
}

uint8_t PowerChannel_IsEnabled(void)
{
    return channel_enabled;
}
