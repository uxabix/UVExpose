/*
 * buzzer.h
 * Buzzer service - centralized audio notification management
 *
 *  Created on: Feb 21, 2026
 *      Author: kiril
 */

#ifndef INC_SERVICES_BUZZER_H_
#define INC_SERVICES_BUZZER_H_

#include <stdint.h>

/**
 * Buzzer modes (settings from menu_expose_options)
 */
typedef enum {
    BUZZER_MODE_SILENT = 0,           // No sound
    BUZZER_MODE_BEEP_ONCE,             // Single beep on timer finish
    BUZZER_MODE_BEEP_UNTIL_STOP        // Continuous beeping until user stops
} buzzer_mode_t;

/**
 * Initialize the buzzer service (GPIO configuration)
 */
void Buzzer_Init(void);

/**
 * Set buzzer operation mode
 * @param mode The buzzer mode (from menu_expose_options)
 */
void Buzzer_SetMode(buzzer_mode_t mode);

/**
 * Configure beep pattern for BUZZER_MODE_BEEP_ONCE mode
 * @param count Number of beeps to play (1-10)
 * @param beep_duration_ms Duration of each beep in milliseconds
 * @param period_ms Period between beeps in milliseconds (silence between beeps)
 */
void Buzzer_SetBeepPattern(uint8_t count, uint16_t beep_duration_ms, uint16_t period_ms);

/**
 * Notify buzzer that exposure timer has finished
 * Buzzer will beep according to the configured mode
 */
void Buzzer_NotifyTimerFinished(void);

/**
 * Stop buzzer immediately (user pressed button or stopped exposure)
 */
void Buzzer_Stop(void);

/**
 * Main processing function - must be called regularly (e.g., every 10-100ms)
 * Updates beep timing and state machine
 */
void Buzzer_Process(void);

#endif /* INC_SERVICES_BUZZER_H_ */
