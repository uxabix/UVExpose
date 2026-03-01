/*
 * battery_service.c
 *
 *  Created on: Feb 22, 2026
 *      Author: Gemini
 */

#include "Services/battery_service.h"
#include "Services/adc_service.h" // Use the new ADC service
#include "config.h"

#if (BATTERY_SERVICE_ENABLED == 1)

#include <stdbool.h>

// For power-of-2 sample counts, we can use bit shifting for division.
#if (BATTERY_FILTER_SAMPLES_COUNT & (BATTERY_FILTER_SAMPLES_COUNT - 1)) != 0
#error "BATTERY_FILTER_SAMPLES_COUNT must be a power of 2 (e.g., 2, 4, 8, 16)"
#endif

// Static variables to hold the service's state
static bool s_is_initialized = false;
static bool s_is_in_critical_hysteresis = false;
static bool s_is_exposure_locked = false;

// Last known filtered values
static uint16_t s_voltage_mv = 0;
static uint8_t s_percentage = 0;
static BatteryStatus_t s_status = BATTERY_STATUS_UNKNOWN;

// For moving average filter
static uint32_t s_filter_buffer[BATTERY_FILTER_SAMPLES_COUNT] = {0};
static uint8_t s_filter_index = 0;
static uint32_t s_filter_sum = 0;

/**
 * @brief Updates the moving average filter with a new sample.
 */
static uint16_t _update_filter(uint16_t new_sample) {
    // Subtract the oldest value from the sum
    s_filter_sum -= s_filter_buffer[s_filter_index];
    // Add the new value
    s_filter_sum += new_sample;
    // Store the new value in the buffer
    s_filter_buffer[s_filter_index] = new_sample;
    // Move to the next index
    s_filter_index = (s_filter_index + 1) % BATTERY_FILTER_SAMPLES_COUNT;

    // Calculate the average
    return s_filter_sum / BATTERY_FILTER_SAMPLES_COUNT;
}


void BatteryService_Init(void) {
    // Pre-fill the filter buffer with initial readings to get a stable start
    uint16_t initial_adc_val = AdcService_ReadChannel(BATTERY_ADC_CHANNEL);
    s_filter_sum = 0;
    for (int i = 0; i < BATTERY_FILTER_SAMPLES_COUNT; i++) {
        s_filter_buffer[i] = initial_adc_val;
        s_filter_sum += initial_adc_val;
    }
    s_filter_index = 0;

    s_is_initialized = true;
    s_is_in_critical_hysteresis = false;
    s_is_exposure_locked = false;

    // Perform one measurement to populate initial values
    BatteryService_Measure();
}

void BatteryService_Measure(void) {
    if (!s_is_initialized) return;

    // 1. Get raw ADC reading from the ADC service
    uint16_t raw_adc = AdcService_ReadChannel(BATTERY_ADC_CHANNEL);

    // 2. Get filtered ADC value
    uint16_t filtered_adc = _update_filter(raw_adc);

    // 3. Convert ADC value to voltage at the divider's output (V_out)
    uint32_t v_out_mv = AdcService_RawToMv(filtered_adc);

    // 4. Calculate actual battery voltage (V_bat) from V_out
    // V_bat = V_out * (R_TOP + R_BOTTOM) / R_BOTTOM
    // To maintain integer math precision, perform multiplication first.
    const uint32_t r_sum = BATTERY_DIVIDER_R_TOP + BATTERY_DIVIDER_R_BOTTOM;
    s_voltage_mv = ((uint64_t)v_out_mv * r_sum) / BATTERY_DIVIDER_R_BOTTOM;

    // 5. Calculate battery percentage (linear mapping)
    if (s_voltage_mv <= BATTERY_MIN_MV) {
        s_percentage = 0;
    } else if (s_voltage_mv >= BATTERY_MAX_MV) {
        s_percentage = 100;
    } else {
        // Use integer arithmetic: ( (current - min) * 100 ) / (max - min)
        s_percentage = (uint8_t)( ((uint32_t)(s_voltage_mv - BATTERY_MIN_MV) * 100) / (BATTERY_MAX_MV - BATTERY_MIN_MV) );
    }

    // 6. Update exposure lock state with hysteresis
    if (s_is_exposure_locked) {
        if (s_voltage_mv > (BATTERY_EXPOSURE_LOCK_MV + BATTERY_HYSTERESIS_MV)) {
            s_is_exposure_locked = false;
        }
    } else {
        if (s_voltage_mv <= BATTERY_EXPOSURE_LOCK_MV) {
            s_is_exposure_locked = true;
        }
    }

    // 7. Update battery status with hysteresis for the critical level
    uint16_t critical_threshold = BATTERY_CRITICAL_MV;

    if (s_is_in_critical_hysteresis) {
        // If we were in a critical state, we need to recover past the hysteresis threshold
        if (s_voltage_mv > (critical_threshold + BATTERY_HYSTERESIS_MV)) {
            s_is_in_critical_hysteresis = false; // We have recovered
        }
    } else {
        // If not in hysteresis, check if we should enter the critical state
        if (s_voltage_mv <= critical_threshold) {
            s_is_in_critical_hysteresis = true;
        }
    }

    // Now, determine the final status
    if (s_is_in_critical_hysteresis) {
        s_status = BATTERY_STATUS_CRITICAL;
    } else if (s_voltage_mv <= BATTERY_WARNING_MV) {
        s_status = BATTERY_STATUS_WARNING;
    } else {
        s_status = BATTERY_STATUS_OK;
    }
}

uint16_t BatteryService_GetVoltageMv(void) {
    return s_voltage_mv;
}

uint8_t BatteryService_GetPercentage(void) {
    return s_percentage;
}

BatteryStatus_t BatteryService_GetStatus(void) {
    return s_status;
}

uint8_t BatteryService_IsExposureAllowed(void) {
#if (IGNORE_BATTERY_LOW == 1)
    return 1u;
#else
    return s_is_exposure_locked ? 0u : 1u;
#endif
}

#endif // BATTERY_SERVICE_ENABLED
