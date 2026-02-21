/*
 * power_channel.h
 * Hardware abstraction layer for UV LED power control (transistor/relay)
 *
 *  Created on: Feb 21, 2026
 *      Author: kiril
 */

#ifndef INC_APP_POWER_CHANNEL_H_
#define INC_APP_POWER_CHANNEL_H_

#include <stdint.h>

/**
 * Initialize the power channel (GPIO pin configuration)
 */
void PowerChannel_Init(void);

/**
 * Enable the UV LED (turn on transistor)
 */
void PowerChannel_Enable(void);

/**
 * Disable the UV LED (turn off transistor)
 */
void PowerChannel_Disable(void);

/**
 * Get current state of the power channel
 * @return 1 if enabled, 0 if disabled
 */
uint8_t PowerChannel_IsEnabled(void);

#endif /* INC_APP_POWER_CHANNEL_H_ */
