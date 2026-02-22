/*
 * exposure_service.h
 * Application layer exposure control service
 * Manages UV LED exposure timing, states, and power control
 *
 *  Created on: Feb 21, 2026
 *      Author: kiril
 */

#ifndef INC_APP_EXPOSURE_SERVICE_H_
#define INC_APP_EXPOSURE_SERVICE_H_

#include <stdint.h>

/**
 * Exposure service states
 */
typedef enum {
    EXPOSURE_STATE_STOPPED,         // Not running
    EXPOSURE_STATE_RUNNING,         // Active exposure
    EXPOSURE_STATE_PAUSED,          // Paused, can resume
    EXPOSURE_STATE_FINISHED,        // Time expired (normal mode)
    EXPOSURE_STATE_FINISHED_UNTIL_OFF // Time expired (until-off mode)
} exposure_state_t;

/**
 * Initialize the exposure service
 */
void Exposure_Init(void);

/**
 * Main processing function - must be called regularly (e.g., every 10-100ms)
 * Updates timer and transitions states
 */
void Exposure_Process(void);

/**
 * Start exposure with specified parameters
 * @param time_ms Total exposure time in milliseconds (0 = until manual stop)
 * @param until_off_mode 1 = continue after time expires, 0 = stop after time expires
 * @param beep_mode Beep configuration (0=silent, 1=single, 2=multiple)
 */
void Exposure_Start(uint32_t time_ms, uint8_t until_off_mode, uint8_t beep_mode);

/**
 * Pause current exposure (can resume later)
 */
void Exposure_Pause(void);

/**
 * Resume paused exposure
 */
void Exposure_Resume(void);

/**
 * Stop exposure immediately
 */
void Exposure_Stop(void);

/**
 * Get current state of exposure
 * @return Current exposure_state_t
 */
exposure_state_t Exposure_GetState(void);

/**
 * Get remaining time for exposure
 * @return Remaining time in milliseconds (0xFFFFFFFF if unlimited)
 */
uint32_t Exposure_GetRemainingTime(void);

/**
 * Query if exposure is currently running
 * @return 1 if running, 0 otherwise
 */
uint8_t Exposure_IsRunning(void);

/**
 * Query if exposure is paused
 * @return 1 if paused, 0 otherwise
 */
uint8_t Exposure_IsPaused(void);

/**
 * Query if exposure has finished
 * @return 1 if finished (time expired), 0 otherwise
 */
uint8_t Exposure_IsFinished(void);

/**
 * Query if we're in "until off" mode after time expiration
 * @return 1 if in until-off continuation, 0 otherwise
 */
uint8_t Exposure_IsFinishedUntilOff(void);

/**
 * Get the beep mode for current exposure
 * @return Beep mode (0=silent, 1=single, 2=multiple)
 */
uint8_t Exposure_GetBeepMode(void);

/**
 * Query if exposure has error
 * @return 1 if error, 0 otherwise
 */
uint8_t Exposure_HasError(void);

/**
 * Query if exposure timer is active
 * @return 1 if timer active, 0 otherwise
 */
uint8_t Exposure_TimerActive(void);

#endif /* INC_APP_EXPOSURE_SERVICE_H_ */
