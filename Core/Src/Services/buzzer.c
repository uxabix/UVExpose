/*
 * buzzer.c
 * Buzzer service - centralized audio notification management
 *
 *  Created on: Feb 21, 2026
 *      Author: kiril
 */

#include "Services/buzzer.h"
#include "Services/soft_timer.h"
#include "Services/settings_service.h"
#include "gpio.h"
#include "main.h"
#include "config.h"

/**
 * Internal buzzer states
 */
typedef enum {
    BUZZER_IDLE = 0,        // Not active
    BUZZER_SINGLE_BEEP,     // Playing single beep
    BUZZER_REPEAT_BEEP,     // Repeating beep pattern
    BUZZER_BEEP_PAUSE       // Pause between beeps in multi-beep mode
} buzzer_state_t;

// Service state
static buzzer_mode_t current_mode = BUZZER_MODE_SILENT;
static buzzer_state_t current_state = BUZZER_IDLE;
static soft_timer_t beep_timer;

// Timing parameters (in milliseconds) - imported from config.h
// #define REPEAT_BEEP_PERIOD is now BUZZER_REPEAT_BEEP_PERIOD
// #define REPEAT_BEEP_ON_TIME is now BUZZER_REPEAT_BEEP_ON_TIME

// Multi-beep pattern settings (for BUZZER_MODE_BEEP_ONCE)
static uint8_t beep_count = SETTINGS_DEFAULT_BEEP_COUNT;                   // Number of beeps to play
static uint16_t beep_duration = SETTINGS_DEFAULT_BEEP_DURATION_MS;         // Duration of each beep
static uint16_t beep_period = SETTINGS_DEFAULT_BEEP_PERIOD_MS;             // Period (silence) between beeps

// Multi-beep runtime tracking
static uint8_t beep_counter = 0;               // How many beeps have been played

// ==== Forward declarations (private functions) ====
static void buzzer_on(void);
static void buzzer_off(void);
static void buzzer_toggle(void);

void Buzzer_Init(void)
{
    // GPIO already initialized in MX_GPIO_Init()
    // Just ensure buzzer is OFF on startup
    buzzer_off();
    current_state = BUZZER_IDLE;
    current_mode = BUZZER_MODE_SILENT;
    beep_counter = 0;
    
    // Initialize pattern from persisted settings (with clamping).
    Buzzer_SetBeepPattern(g_settings.beep_count, g_settings.beep_duration, g_settings.beep_period);
}

static void buzzer_on(void)
{
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, BUZZER_ACTIVE_LEVEL);
}

static void buzzer_off(void)
{
    // Set to the inverse of active level
    uint8_t off_level = (BUZZER_ACTIVE_LEVEL == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, off_level);
}

static void buzzer_toggle(void)
{
    HAL_GPIO_TogglePin(Buzzer_GPIO_Port, Buzzer_Pin);
}

void Buzzer_SetMode(buzzer_mode_t mode)
{
    current_mode = mode;
}

void Buzzer_SetBeepPattern(uint8_t count, uint16_t beep_duration_ms, uint16_t period_ms)
{
    // Validate and clamp parameters
    if (count < 1) count = 1;
    if (count > 10) count = 10;  // Maximum 10 beeps
    
    if (beep_duration_ms < 50) beep_duration_ms = 50;    // Minimum 50ms
    if (beep_duration_ms > 1000) beep_duration_ms = 1000; // Maximum 1000ms
    
    if (period_ms < 50) period_ms = 50;    // Minimum 50ms between beeps
    if (period_ms > 2000) period_ms = 2000; // Maximum 2000ms between beeps
    
    beep_count = count;
    beep_duration = beep_duration_ms;
    beep_period = period_ms;
}

void Buzzer_NotifyTimerFinished(void)
{
    switch (current_mode) {
        case BUZZER_MODE_SILENT:
            // No action needed
            break;

        case BUZZER_MODE_BEEP_ONCE:
            // Play multiple beeps with configured pattern
            beep_counter = 0;  // Reset beep counter
            buzzer_on();
            soft_timer_start(&beep_timer, beep_duration);
            current_state = BUZZER_SINGLE_BEEP;
            break;

        case BUZZER_MODE_BEEP_UNTIL_STOP:
            // Start continuous beeping
            buzzer_on();
            soft_timer_start(&beep_timer, BUZZER_REPEAT_BEEP_ON_TIME);
            current_state = BUZZER_REPEAT_BEEP;
            break;
    }
}

void Buzzer_Stop(void)
{
    buzzer_off();
    beep_counter = 0;
    current_state = BUZZER_IDLE;
}

void Buzzer_Process(void)
{
    if (!soft_timer_expired(&beep_timer)) {
        return;
    }

    switch (current_state) {
        case BUZZER_SINGLE_BEEP:
            // Check if we need another beep
            beep_counter++;
            
            if (beep_counter < beep_count) {
                // More beeps needed - turn off and wait for pause period
                buzzer_off();
                soft_timer_start(&beep_timer, beep_period);
                current_state = BUZZER_BEEP_PAUSE;
            } else {
                // All beeps completed
                buzzer_off();
                current_state = BUZZER_IDLE;
            }
            break;

        case BUZZER_BEEP_PAUSE:
            // Pause finished, play next beep
            buzzer_on();
            soft_timer_start(&beep_timer, beep_duration);
            current_state = BUZZER_SINGLE_BEEP;
            break;

        case BUZZER_REPEAT_BEEP:
            // Toggle between on and off
            buzzer_toggle();
            // Restart timer for next toggle
            // If currently ON, wait BUZZER_REPEAT_BEEP_ON_TIME before turning off
            // If currently OFF, wait BUZZER_REPEAT_BEEP_PERIOD - BUZZER_REPEAT_BEEP_ON_TIME before turning on
            uint32_t next_wait = (HAL_GPIO_ReadPin(Buzzer_GPIO_Port, Buzzer_Pin) == BUZZER_ACTIVE_LEVEL)
                                    ? BUZZER_REPEAT_BEEP_ON_TIME
                                    : (BUZZER_REPEAT_BEEP_PERIOD - BUZZER_REPEAT_BEEP_ON_TIME);
            soft_timer_start(&beep_timer, next_wait);
            break;

        case BUZZER_IDLE:
        default:
            // No active beeping, nothing to do
            break;
    }
}

