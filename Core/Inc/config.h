/*
 * config.h
 * Central configuration file for UVExpose project
 * All important settings are centralized here
 *
 *  Created on: Feb 21, 2026
 *      Author: kiril
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * GPIO OUTPUT LEVELS CONFIGURATION
 * ============================================================================ */

/**
 * LED Output level when ON/active
 * GPIO_PIN_SET (1) = active high
 * GPIO_PIN_RESET (0) = active low
 */
#define LED_OUTPUT_ACTIVE_LEVEL    GPIO_PIN_SET

/**
 * Buzzer level when ON/active
 * GPIO_PIN_SET (1) = active high
 * GPIO_PIN_RESET (0) = active low
 */
#define BUZZER_ACTIVE_LEVEL        GPIO_PIN_SET

/* ============================================================================
 * SCREEN CONFIGURATION (SSD1306 OLED)
 * ============================================================================ */

/**
 * I2C address of SSD1306 display
 * Typical values: 0x78 or 0x7A (depends on SA0 pin)
 */
#define SCREEN_I2C_ADDR            0x78

/**
 * Screen dimensions in pixels
 */
#define SCREEN_WIDTH               128
#define SCREEN_HEIGHT              64

/**
 * Top bar configuration (status area)
 */
#define SCREEN_TOP_BAR_HEIGHT      9
#define SCREEN_TOP_BAR_PADDING     1

/**
 * Battery indicator position and size
 */
#define SCREEN_BATTERY_POS_X       0
#define SCREEN_BATTERY_POS_Y       0
#define SCREEN_BATTERY_SEGMENT_WIDTH   4
#define SCREEN_BATTERY_SEGMENT_HEIGHT  6
#define SCREEN_BATTERY_PROTECTION_STEP_X  5
#define SCREEN_BATTERY_PROTECTION_STEP_Y  1

/**
 * Menu display configuration
 */
#define SCREEN_MENU_HIGHLIGHT_PADDING_X  2
#define SCREEN_MENU_HIGHLIGHT_PADDING_Y  1
#define SCREEN_MENU_SCROLLBAR_WIDTH      4
#define SCREEN_MENU_SCROLLBAR_MARGIN     1  // From right side
#define SCREEN_MENU_TEXT_HEIGHT          10
#define SCREEN_MENU_TEXT_WIDTH           7
#define SCREEN_MENU_PADDING              1

/**
 * Timer display configuration
 */
#define SCREEN_TIMER_TEXT_HEIGHT   26
#define SCREEN_TIMER_TEXT_WIDTH    16
#define SCREEN_TIMER_FLOAT_STEP_X  1   // Pixels to move per update
#define SCREEN_TIMER_FLOAT_STEP_Y  1   // Pixels to move per update
#define SCREEN_TIMER_FLOAT_MARGIN  5   // Margin from display area edges

/* ============================================================================
 * BUZZER TIMINGS CONFIGURATION
 * ============================================================================ */

/**
 * Single beep duration when user triggers a beep
 * in milliseconds
 */
#define BUZZER_SINGLE_BEEP_DURATION    300

/**
 * Repeated beep period (complete on+off cycle)
 * in milliseconds
 */
#define BUZZER_REPEAT_BEEP_PERIOD      500

/**
 * Duration of buzzer ON in repeated beep mode
 * in milliseconds
 */
#define BUZZER_REPEAT_BEEP_ON_TIME     200

/**
 * Default number of beeps for beep pattern mode
 */
#define BUZZER_DEFAULT_BEEP_COUNT      1

/**
 * Default period (silence between beeps) for multi-beep pattern
 * in milliseconds
 */
#define BUZZER_DEFAULT_BEEP_PERIOD     200

/* ============================================================================
 * ENCODER CONFIGURATION
 * ============================================================================ */

/**
 * Debounce time for encoder button (if needed in future)
 * in milliseconds
 */
#define ENCODER_BUTTON_DEBOUNCE_MS     20

/* ============================================================================
 * PROTECTION AND SAFETY SETTINGS
 * ============================================================================ */

/**
 * Hall sensor pin active level when lid is considered "open".
 * Use GPIO_PIN_SET for active-high, GPIO_PIN_RESET for active-low.
 * Default: active-high (GPIO_PIN_SET)
 */
#define LID_HALL_ACTIVE_LEVEL          GPIO_PIN_SET

/**
 * Enable lid-open protection by default. UI option in settings overrides this at runtime.
 */
#define ENABLE_LID_PROTECTION_DEFAULT  1

/* -------------------------------------------------------------------------- */
/* Linear Hall / ADC configuration                                              */
/* -------------------------------------------------------------------------- */

/**
 * Use ADC for reading linear Hall sensor (1 = ADC, 0 = digital GPIO)
 * If set to 1, ensure an ADC channel and GPIO in analog mode is configured
 * in CubeMX and the HAL ADC module is enabled.
 */
#define LID_HALL_USE_ADC               1

/**
 * ADC channel for the linear Hall sensor.
 * Make sure this corresponds to the channel configured in CubeMX.
 * NOTE: This is a HAL definition, ensure it's correct for your setup.
 */
#define LID_HALL_ADC_CHANNEL           ADC_CHANNEL_9 // Example, PLEASE ADJUST

/** ADC reference voltage (millivolts) and ADC resolution (max code) */
#define ADC_VREF_MV                    3300u
#define ADC_RESOLUTION_MAX             4095u

/**
 * Voltage divider present on sensor output (0 = no divider, 1 = divider present)
 * If present, configure `DIVIDER_R_TOP` and `DIVIDER_R_BOTTOM` in ohms.
 */
#define LID_HALL_VOLTAGE_DIVIDER_PRESENT  1
#define DIVIDER_R_TOP                  68000u /* Rtop in ohms */
#define DIVIDER_R_BOTTOM               68000u /* Rbottom in ohms */

/**
 * Default thresholds for linear Hall sensor in millivolts.
 * `OPEN` should be higher than `CLOSE` for typical sensors (hysteresis)
 */
#define LID_HALL_OPEN_THRESHOLD_MV     2000u
#define LID_HALL_CLOSE_THRESHOLD_MV    1800u

/* ============================================================================
 * POWER MANAGEMENT
 * ============================================================================ */

/**
 * Idle timeout before display dims/turns off
 * in milliseconds (0 = disabled)
 */
#define IDLE_TIMEOUT_MS                60000  // 60 seconds

/**
 * Auto-dimming brightness level (0-255)
 * when idle
 */
#define IDLE_BRIGHTNESS_LEVEL          50

/* ============================================================================
 * BATTERY MONITORING CONFIGURATION
 * ============================================================================ */

/**
 * Enable Battery Service (1 = enabled, 0 = disabled)
 * If disabled, the service will not be compiled, and all related checks
 * will be bypassed.
 */
#define BATTERY_SERVICE_ENABLED 1

/**
 * ADC channel for battery voltage measurement.
 * Make sure this corresponds to the channel configured in CubeMX for the battery pin.
 * Example: ADC_CHANNEL_0, ADC_CHANNEL_1, etc.
 * NOTE: This is a HAL definition, ensure it's correct for your setup.
 */
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_8 // Example, PLEASE ADJUST

/* -------------------------------------------------------------------------- */
/* Voltage Divider Configuration                                              */
/* -------------------------------------------------------------------------- */
/**
 * The battery voltage is measured through a voltage divider.
 * Vout = Vbat * (R_BOTTOM / (R_TOP + R_BOTTOM))
 * Provide the resistor values. Using identical values simplifies math.
 * For higher precision with integers, you can use scaled values (e.g., kOhms).
 */
#define BATTERY_DIVIDER_R_TOP     10000u /* R_top in ohms */
#define BATTERY_DIVIDER_R_BOTTOM  10000u /* R_bottom in ohms */

/* -------------------------------------------------------------------------- */
/* Battery Characteristics (in Millivolts)                                    */
/* -------------------------------------------------------------------------- */
/**
 * These values define the battery's voltage range for percentage calculation.
 * This assumes a linear discharge curve. All values are in millivolts.
 */
#define BATTERY_MAX_MV            4200u // Voltage of a fully charged battery (e.g., Li-Ion)
#define BATTERY_MIN_MV            3000u // Voltage of a fully discharged battery (cutoff)

/**
 * Voltage thresholds for safety actions.
 */
#define BATTERY_WARNING_MV        3300u // Level to show a warning to the user
#define BATTERY_CRITICAL_MV       3100u // Level to force shutdown/stop operations

/**
 * Software hysteresis to prevent rapid state changes near a threshold.
 * When in a critical state, the voltage must rise above (THRESHOLD + HYSTERESIS)
 * to be considered normal again. In millivolts.
 */
#define BATTERY_HYSTERESIS_MV     150u

/* -------------------------------------------------------------------------- */
/* ADC Filtering Configuration                                                */
/* -------------------------------------------------------------------------- */
/**
 * Number of ADC samples for the moving average filter.
 * A larger value gives more stability but slower response.
 * MUST be a power of 2 for efficient integer arithmetic (e.g., 2, 4, 8, 16).
 */
#define BATTERY_FILTER_SAMPLES_COUNT 8

/* ============================================================================
 * SETTINGS STORAGE CONFIGURATION
 * ============================================================================ */
/**
 * Flash address for settings storage (last page)
 * Adjust for your STM32 model
 */
#define SETTINGS_FLASH_ADDR 0x08007C00 // Last 1KB page for STM32F103C6 (32KB flash)
#define SETTINGS_FLASH_PAGE_SIZE 1024

/**
 * Maximum number of presets
 */
#define SETTINGS_MAX_PRESETS 5

#ifdef __cplusplus
}
#endif

#endif /* INC_CONFIG_H_ */
