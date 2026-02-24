/*
 * battery_service.h
 *
 *  Created on: Feb 22, 2026
 *      Author: Kiril
 *
 *  Manages battery voltage measurement, filtering, and state-of-charge estimation.
 */

#ifndef INC_SERVICES_BATTERY_SERVICE_H_
#define INC_SERVICES_BATTERY_SERVICE_H_

#include "main.h"
#include "config.h"
#include <stdint.h>

// Forward declaration of the ADC handle type
struct ADC_HandleTypeDef;

#if (BATTERY_SERVICE_ENABLED == 1)

/**
 * @brief Battery status enumeration.
 * Reflects the current state of the battery based on configured thresholds.
 */
typedef enum {
    BATTERY_STATUS_OK,       // Voltage is above the warning level.
    BATTERY_STATUS_WARNING,  // Voltage is between warning and critical levels.
    BATTERY_STATUS_CRITICAL, // Voltage is below the critical level.
    BATTERY_STATUS_UNKNOWN   // Initial state before first measurement.
} BatteryStatus_t;

/**
 * @brief Initializes the battery service.
 * Must be called once at startup.
 */
void BatteryService_Init(void);

/**
 * @brief Performs a new battery measurement and updates internal state.
 * This function should be called periodically (e.g., via a software timer or in the main loop).
 */
void BatteryService_Measure(void);

/**
 * @brief Gets the current estimated battery voltage.
 *
 * @return Battery voltage in millivolts (mV).
 */
uint16_t BatteryService_GetVoltageMv(void);

/**
 * @brief Gets the current estimated state of charge (SoC).
 *
 * @return Battery charge level from 0 to 100.
 */
uint8_t BatteryService_GetPercentage(void);

/**
 * @brief Gets the current battery status.
 *
 * @return BatteryStatus_t enumeration value.
 */
BatteryStatus_t BatteryService_GetStatus(void);

#else // BATTERY_SERVICE_ENABLED == 0

// If the service is disabled, define dummy (inline) functions to avoid compilation errors
// in other modules that might unconditionally call these functions.
// These functions do nothing or return safe default values.

typedef enum {
    BATTERY_STATUS_OK
} BatteryStatus_t;


static inline void BatteryService_Init(void) { }
static inline void BatteryService_Measure(void) { }
static inline uint16_t BatteryService_GetVoltageMv(void) { return 4200; /* Return a safe, full-battery value */ }
static inline uint8_t BatteryService_GetPercentage(void) { return 100; }
static inline BatteryStatus_t BatteryService_GetStatus(void) { return BATTERY_STATUS_OK; }

#endif // BATTERY_SERVICE_ENABLED

#endif /* INC_SERVICES_BATTERY_SERVICE_H_ */
