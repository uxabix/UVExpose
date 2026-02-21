/*
 * exposure_service.c
 * Application layer exposure control implementation
 *
 *  Created on: Feb 21, 2026
 *      Author: kiril
 */

#include "App/exposure_service.h"
#include "App/power_channel.h"
#include "Services/soft_timer.h"

// Service state machine
static exposure_state_t state = EXPOSURE_STATE_STOPPED;

// Configuration
static uint32_t total_time_ms = 0;      // Total time set by user
static uint8_t until_off_mode = 0;      // Continue after timer expires
static uint8_t beep_mode = 0;           // Beep configuration

// Runtime
static uint32_t remaining_time_ms = 0;  // Countdown timer
static soft_timer_t process_timer;      // Internal tick timer

#define PROCESS_TICK_MS 100              // Update every 100ms

void Exposure_Init(void)
{
    PowerChannel_Init();
    state = EXPOSURE_STATE_STOPPED;
    remaining_time_ms = 0;
    total_time_ms = 0;
    until_off_mode = 0;
    beep_mode = 0;
}

void Exposure_Process(void)
{
    // Only process if running or paused (not stopped)
    if (state == EXPOSURE_STATE_STOPPED || state == EXPOSURE_STATE_FINISHED) {
        return;
    }

    // Check if we need to decrement timer
    if (state == EXPOSURE_STATE_RUNNING && total_time_ms > 0) {
        if (soft_timer_expired(&process_timer)) {
            // Decrement by tick interval
            if (remaining_time_ms >= PROCESS_TICK_MS) {
                remaining_time_ms -= PROCESS_TICK_MS;
            } else {
                remaining_time_ms = 0;

                // Timer finished - transition to appropriate finished state
                if (until_off_mode) {
                    // Continue LEDs on, but mark as finished
                    state = EXPOSURE_STATE_FINISHED_UNTIL_OFF;
                } else {
                    // Turn off immediately
                    PowerChannel_Disable();
                    state = EXPOSURE_STATE_FINISHED;
                }
            }

            soft_timer_start(&process_timer, PROCESS_TICK_MS);
        }
    }
}

void Exposure_Start(uint32_t time_ms, uint8_t until_off_mode_param, uint8_t beep_mode_param)
{
    // Store configuration
    total_time_ms = time_ms;
    until_off_mode = until_off_mode_param;
    beep_mode = beep_mode_param;
    remaining_time_ms = time_ms;

    // Turn on power channel
    PowerChannel_Enable();

    // Start the process timer if we have a time limit
    if (time_ms > 0) {
        soft_timer_start(&process_timer, PROCESS_TICK_MS);
        state = EXPOSURE_STATE_RUNNING;
    } else {
        // Unlimited mode - no timer, just run until stop
        state = EXPOSURE_STATE_RUNNING;
        remaining_time_ms = 0xFFFFFFFF; // Use as marker for "unlimited"
    }
}

void Exposure_Pause(void)
{
    if (state == EXPOSURE_STATE_RUNNING) {
        state = EXPOSURE_STATE_PAUSED;
        PowerChannel_Disable();
    }
}

void Exposure_Resume(void)
{
    if (state == EXPOSURE_STATE_PAUSED) {
        if (total_time_ms > 0) {
            soft_timer_start(&process_timer, PROCESS_TICK_MS);
        }
        state = EXPOSURE_STATE_RUNNING;
        PowerChannel_Enable();
    }
}

void Exposure_Stop(void)
{
    PowerChannel_Disable();
    state = EXPOSURE_STATE_STOPPED;
    remaining_time_ms = 0;
}

exposure_state_t Exposure_GetState(void)
{
    return state;
}

uint32_t Exposure_GetRemainingTime(void)
{
    return remaining_time_ms;
}

uint8_t Exposure_IsRunning(void)
{
    return (state == EXPOSURE_STATE_RUNNING);
}

uint8_t Exposure_IsPaused(void)
{
    return (state == EXPOSURE_STATE_PAUSED);
}

uint8_t Exposure_IsFinished(void)
{
    return (state == EXPOSURE_STATE_FINISHED);
}

uint8_t Exposure_IsFinishedUntilOff(void)
{
    return (state == EXPOSURE_STATE_FINISHED_UNTIL_OFF);
}
