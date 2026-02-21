/**
 * ============================================================================
 * CONFIGURATION SYSTEM - UVExpose Project
 * ============================================================================
 * 
 * This document explains the centralized configuration system introduced
 * with config.h
 * 
 * ============================================================================
 * OVERVIEW
 * ============================================================================
 * 
 * config.h is the single source of truth for all project-wide settings.
 * Instead of having hardcoded values scattered throughout the codebase,
 * all important parameters are now centralized in one place.
 * 
 * This makes the project easier to:
 *   - Customize for different hardware configurations
 *   - Adjust parameters without searching through multiple files
 *   - Maintain consistency across different modules
 *   - Add new configurable parameters in the future
 * 
 * ============================================================================
 * CURRENT CONFIGURATION SECTIONS
 * ============================================================================
 * 
 * 1. GPIO OUTPUT LEVELS CONFIGURATION
 *    - LED_OUTPUT_ACTIVE_LEVEL: When GPIO level turns LED on (GPIO_PIN_SET or GPIO_PIN_RESET)
 *    - BUZZER_ACTIVE_LEVEL: When GPIO level activates buzzer (GPIO_PIN_SET or GPIO_PIN_RESET)
 * 
 * 2. SCREEN CONFIGURATION (SSD1306 OLED)
 *    - SCREEN_I2C_ADDR: I2C address of the display
 *    - SCREEN_WIDTH / SCREEN_HEIGHT: Display dimensions
 *    - SCREEN_TOP_BAR_*: Status bar parameters
 *    - SCREEN_BATTERY_*: Battery indicator position and size
 *    - SCREEN_MENU_*: Menu display parameters (fonts, padding, scrollbar)
 *    - SCREEN_TIMER_*: Timer display parameters (dimensions, movement)
 * 
 * 3. BUZZER TIMINGS CONFIGURATION
 *    - BUZZER_SINGLE_BEEP_DURATION: Duration of a single beep (milliseconds)
 *    - BUZZER_REPEAT_BEEP_PERIOD: Full cycle period of repeated beeps (milliseconds)
 *    - BUZZER_REPEAT_BEEP_ON_TIME: Duration buzzer is ON in repeat mode (milliseconds)
 *    - BUZZER_DEFAULT_BEEP_COUNT: Default number of beeps for pattern mode
 *    - BUZZER_DEFAULT_BEEP_PERIOD: Default silence duration between beeps (milliseconds)
 * 
 * 4. ENCODER CONFIGURATION
 *    - ENCODER_BUTTON_DEBOUNCE_MS: Button debounce time (for future use)
 * 
 * 5. PROTECTION AND SAFETY SETTINGS
 *    - ENABLE_BURNOUT_PROTECTION: Enable/disable burnout protection mode
 * 
 * 6. POWER MANAGEMENT
 *    - IDLE_TIMEOUT_MS: Time before display dims
 *    - IDLE_BRIGHTNESS_LEVEL: Brightness level when idle (0-255)
 * 
 * ============================================================================
 * FILES UPDATED TO USE config.h
 * ============================================================================
 * 
 * 1. Core/Src/Services/buzzer.c
 *    - Now uses BUZZER_* constants for all timing values
 *    - Uses BUZZER_ACTIVE_LEVEL for GPIO control
 * 
 * 2. Core/Src/Display/display.c
 *    - Now uses SCREEN_* constants for all display configuration
 *    - Synchronized with config.h for screen dimensions and layout
 * 
 * 3. Core/Inc/Display/ssd1306.h
 *    - Now uses config.h for default I2C address and screen dimensions
 *    - Maintains #ifndef guards for override compatibility
 * 
 * 4. Core/Src/App/power_channel.c
 *    - Now uses LED_OUTPUT_ACTIVE_LEVEL for GPIO control
 *    - Automatically calculates inactive level for complete abstraction
 * 
 * ============================================================================
 * HOW TO USE / MODIFY CONFIGURATION
 * ============================================================================
 * 
 * To change a setting:
 *   1. Open Core/Inc/config.h
 *   2. Find the relevant section
 *   3. Modify the #define value
 *   4. Save the file
 *   5. Rebuild the project
 * 
 * The change will automatically propagate to all modules that depend on
 * that configuration value.
 * 
 * Examples:
 * 
 *   Change display dimensions:
 *     #define SCREEN_WIDTH               256   // From 128
 *     #define SCREEN_HEIGHT              128   // From 64
 * 
 *   Change LED active level (e.g., if LED is active low):
 *     #define LED_OUTPUT_ACTIVE_LEVEL    GPIO_PIN_RESET  // From GPIO_PIN_SET
 * 
 *   Change buzzer beep duration:
 *     #define BUZZER_SINGLE_BEEP_DURATION 500  // From 300
 * 
 * ============================================================================
 * ADDING NEW CONFIGURATION PARAMETERS
 * ============================================================================
 * 
 * When adding new configurable parameters:
 * 
 * 1. Add the #define to config.h in the appropriate section
 *    (or create a new section if needed)
 * 
 * 2. Document what the parameter does with clear comments
 * 
 * 3. Choose a descriptive name that indicates its purpose and value type
 *    Naming convention:
 *      - Boolean flags: ENABLE_* or DISABLE_*
 *      - Dimensions/sizes: *_WIDTH, *_HEIGHT, *_SIZE
 *      - Timing values: *_MS, *_TIMEOUT, *_DELAY
 *      - Levels/values: *_LEVEL, *_VALUE
 * 
 * 4. Update the documentation section of this file
 * 
 * 5. Include #include "config.h" in any file that uses the new parameter
 * 
 * Example:
 *   In config.h:
 *     // New feature configuration
 *     #define NEW_FEATURE_ENABLED            1
 *     #define NEW_FEATURE_TIMEOUT_MS         5000
 * 
 *   In your_file.c:
 *     #include "config.h"
 *     // Then use: NEW_FEATURE_ENABLED and NEW_FEATURE_TIMEOUT_MS
 * 
 * ============================================================================
 * FILE STRUCTURE
 * ============================================================================
 * 
 * Core/Inc/config.h
 *   ├── GPIO OUTPUT LEVELS CONFIGURATION
 *   ├── SCREEN CONFIGURATION
 *   ├── BUZZER TIMINGS CONFIGURATION
 *   ├── ENCODER CONFIGURATION
 *   ├── PROTECTION AND SAFETY SETTINGS
 *   └── POWER MANAGEMENT
 * 
 * ============================================================================
 * BEST PRACTICES
 * ============================================================================
 * 
 * 1. Keep configuration values in config.h, not in individual source files
 * 
 * 2. Include #include "config.h" near the top of any file that uses config parameters
 * 
 * 3. Document configuration values with clear comments explaining:
 *    - What the parameter controls
 *    - Typical/recommended values
 *    - Units (milliseconds, pixels, etc.)
 *    - Valid range if applicable
 * 
 * 4. Use symbolic constants consistently throughout the codebase
 *    (Don't use hardcoded values if a config constant exists)
 * 
 * 5. When making hardware changes (different screen, buzzer, etc.),
 *    update only config.h - the rest of the code should work automatically
 * 
 * ============================================================================
 */
