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

#define DEBUG 0

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
#define SCREEN_TOP_BAR_HEIGHT      17
#define SCREEN_TOP_BAR_PADDING     1

/**
 * Battery indicator position and size
 */
#define SCREEN_BATTERY_POS_X       0
#define SCREEN_BATTERY_POS_Y       0
#define SCREEN_BATTERY_SEGMENT_WIDTH   5
#define SCREEN_BATTERY_SEGMENT_HEIGHT  8
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
#define SCREEN_MENU_BURNIN_SHIFT_MAX_X   2
#define SCREEN_MENU_BURNIN_SHIFT_MAX_Y   2
#define SCREEN_MENU_BURNIN_STEP_MS       5000u

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
 * Repeated beep period (complete on+off cycle)
 * in milliseconds
 */
#define BUZZER_REPEAT_BEEP_PERIOD      500

/**
 * Duration of buzzer ON in repeated beep mode
 * in milliseconds
 */
#define BUZZER_REPEAT_BEEP_ON_TIME     200

/* ============================================================================
 * ENCODER CONFIGURATION
 * ============================================================================ */

/**
 * Debounce time for encoder button (if needed in future)
 * in milliseconds
 */
#define ENCODER_BUTTON_DEBOUNCE_MS     20

/**
 * Encoder direction inversion.
 * 0 = normal direction, 1 = inverted direction.
 */
#define ENCODER_INVERT_DIRECTION        1

/* ============================================================================
 * SETTINGS DEFAULTS (stored in Flash by settings_service)
 * ============================================================================ */

#define SETTINGS_DEFAULT_BURN_IN_PROTECTION     1u
#define SETTINGS_DEFAULT_OPEN_LID_PROTECTION    1u
#define SETTINGS_DEFAULT_BEEP_COUNT             1u
#define SETTINGS_DEFAULT_BEEP_DURATION_MS       300u
#define SETTINGS_DEFAULT_BEEP_PERIOD_MS         200u
#define SETTINGS_DEFAULT_SLEEP_MODE             1u   /* 2 minutes in current menu mapping */
#define SETTINGS_DEFAULT_LID_OPEN_THRESHOLD_MV  2000u
#define SETTINGS_DEFAULT_LID_CLOSE_THRESHOLD_MV 1800u

/* ============================================================================
 * PROTECTION AND SAFETY SETTINGS
 * ============================================================================ */

/**
 * Hall sensor pin active level when lid is considered "open".
 * Use GPIO_PIN_SET for active-high, GPIO_PIN_RESET for active-low.
 * Default: active-high (GPIO_PIN_SET)
 */
#define LID_HALL_ACTIVE_LEVEL          GPIO_PIN_SET

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
 * Global ADC voltage correction in millivolts.
 * Applied in AdcService_RawToMv() for all ADC-based measurements.
 * Positive value increases measured voltage, negative decreases it.
 * Example: if readings are ~220mV too low, set to +220.
 */
#define ADC_GLOBAL_OFFSET_MV           220

/**
 * Voltage divider present on sensor output (0 = no divider, 1 = divider present)
 * If present, configure `DIVIDER_R_TOP` and `DIVIDER_R_BOTTOM` in ohms.
 */
#define LID_HALL_VOLTAGE_DIVIDER_PRESENT  1
#define DIVIDER_R_TOP                  68000u /* Rtop in ohms */
#define DIVIDER_R_BOTTOM               68000u /* Rbottom in ohms */

/* ============================================================================
 * POWER MANAGEMENT
 * ============================================================================ */

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
#define BATTERY_EXPOSURE_LOCK_MV  3100u // Below this level exposure start is blocked
#define BATTERY_CRITICAL_MV       3000u // Level to force sleep

/**
 * Delay before entering sleep on critical battery.
 * During this time UI shows a warning, which also helps with flashing/debug.
 */
#define BATTERY_CRITICAL_SLEEP_DELAY_MS 15000u

/**
 * Debug bypass flags for battery protections.
 * 0 = normal behavior, 1 = ignore protection.
 */
#define IGNORE_BATTERY_LOW       1
#define IGNORE_BATTERY_CRITICAL  1

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

#if (BATTERY_CRITICAL_MV >= BATTERY_EXPOSURE_LOCK_MV)
#error "BATTERY_CRITICAL_MV must be lower than BATTERY_EXPOSURE_LOCK_MV"
#endif

#if (BATTERY_EXPOSURE_LOCK_MV > BATTERY_WARNING_MV)
#error "BATTERY_EXPOSURE_LOCK_MV must be <= BATTERY_WARNING_MV"
#endif

#if ((IGNORE_BATTERY_LOW != 0) && (IGNORE_BATTERY_LOW != 1))
#error "IGNORE_BATTERY_LOW must be 0 or 1"
#endif

#if ((IGNORE_BATTERY_CRITICAL != 0) && (IGNORE_BATTERY_CRITICAL != 1))
#error "IGNORE_BATTERY_CRITICAL must be 0 or 1"
#endif

#if ((SETTINGS_DEFAULT_BURN_IN_PROTECTION != 0) && (SETTINGS_DEFAULT_BURN_IN_PROTECTION != 1))
#error "SETTINGS_DEFAULT_BURN_IN_PROTECTION must be 0 or 1"
#endif

#if ((SETTINGS_DEFAULT_OPEN_LID_PROTECTION != 0) && (SETTINGS_DEFAULT_OPEN_LID_PROTECTION != 1))
#error "SETTINGS_DEFAULT_OPEN_LID_PROTECTION must be 0 or 1"
#endif

#if (SETTINGS_DEFAULT_BEEP_COUNT < 1u) || (SETTINGS_DEFAULT_BEEP_COUNT > 10u)
#error "SETTINGS_DEFAULT_BEEP_COUNT must be in range 1..10"
#endif

#if (SETTINGS_DEFAULT_SLEEP_MODE > 5u)
#error "SETTINGS_DEFAULT_SLEEP_MODE must be in range 0..5"
#endif

#if ((ENCODER_INVERT_DIRECTION != 0) && (ENCODER_INVERT_DIRECTION != 1))
#error "ENCODER_INVERT_DIRECTION must be 0 or 1"
#endif

/* ============================================================================
 * SETTINGS STORAGE CONFIGURATION
 * ============================================================================ */
/**
 * Flash page allocation (STM32F103C6, 1KB pages).
 * Keep settings and presets on dedicated pages.
 */
#define SETTINGS_FLASH_PAGE_SIZE 1024u
#define PRESETS_FLASH_PAGE_SIZE  1024u

/**
 * Last page: settings. Previous page: presets.
 */
#define SETTINGS_FLASH_ADDR 0x08007C00u
#define PRESETS_FLASH_ADDR  0x08007800u

/**
 * Logical UI limit for active presets in RAM and menus.
 */
#define PRESETS_MAX_COUNT   64u

/* Compile-time sanity checks for data pages placement. */
#if ((PRESETS_FLASH_ADDR + PRESETS_FLASH_PAGE_SIZE) > SETTINGS_FLASH_ADDR)
#error "Presets page overlaps settings page"
#endif

#if ((SETTINGS_FLASH_ADDR + SETTINGS_FLASH_PAGE_SIZE) > 0x08008000u)
#error "Settings page is outside STM32F103C6 32KB flash"
#endif

#ifdef __cplusplus
}
#endif

#endif /* INC_CONFIG_H_ */
