# CONFIG_GUIDE

This file documents `Core/Inc/config.h` and how configuration values are used in the firmware.

## 1. Configuration Model

There are three different kinds of configuration in this project:

1. Hardware/static constants
- GPIO active levels, ADC channels, divider values, Flash addresses, screen geometry.
- Changing these directly changes firmware behavior after rebuild.

2. Runtime settings defaults (`SETTINGS_DEFAULT_*`)
- These are initial values for `settings_t` used by `Settings_Init()` only when stored settings are invalid (CRC fail/empty page).
- They do not overwrite existing valid settings in Flash.

3. Debug/override flags
- Compile-time switches such as `DEBUG`, `IGNORE_BATTERY_LOW`, `IGNORE_BATTERY_CRITICAL`.
- Intended for development/testing.

## 2. Key Sections in config.h

## GPIO Output Levels

- `LED_OUTPUT_ACTIVE_LEVEL`
- `BUZZER_ACTIVE_LEVEL`

Define active polarity for output pins.

## Screen Configuration

- `SCREEN_I2C_ADDR`
- `SCREEN_WIDTH`, `SCREEN_HEIGHT`
- Top bar: `SCREEN_TOP_BAR_*`
- Battery icon: `SCREEN_BATTERY_*`
- Menu layout: `SCREEN_MENU_*`
- Menu burn-in movement:
  - `SCREEN_MENU_BURNIN_SHIFT_MAX_X`
  - `SCREEN_MENU_BURNIN_SHIFT_MAX_Y`
  - `SCREEN_MENU_BURNIN_STEP_MS`
- Timer text behavior: `SCREEN_TIMER_*`

## Buzzer Timing (service-level)

- `BUZZER_REPEAT_BEEP_PERIOD`
- `BUZZER_REPEAT_BEEP_ON_TIME`

Pattern defaults for user settings are in `SETTINGS_DEFAULT_*`.

## Encoder

- `ENCODER_BUTTON_DEBOUNCE_MS`
- `ENCODER_INVERT_DIRECTION` (`0` normal, `1` inverted)

## Runtime Settings Defaults

Used when settings in Flash are invalid:

- `SETTINGS_DEFAULT_BURN_IN_PROTECTION`
- `SETTINGS_DEFAULT_OPEN_LID_PROTECTION`
- `SETTINGS_DEFAULT_BEEP_COUNT`
- `SETTINGS_DEFAULT_BEEP_DURATION_MS`
- `SETTINGS_DEFAULT_BEEP_PERIOD_MS`
- `SETTINGS_DEFAULT_SLEEP_MODE`
- `SETTINGS_DEFAULT_LID_OPEN_THRESHOLD_MV`
- `SETTINGS_DEFAULT_LID_CLOSE_THRESHOLD_MV`

## Hall/ADC Configuration

- `LID_HALL_ACTIVE_LEVEL`
- `LID_HALL_USE_ADC`
- `LID_HALL_ADC_CHANNEL`
- `ADC_VREF_MV`, `ADC_RESOLUTION_MAX`
- `ADC_GLOBAL_OFFSET_MV`
- `LID_HALL_VOLTAGE_DIVIDER_PRESENT`
- `DIVIDER_R_TOP`, `DIVIDER_R_BOTTOM`

## Battery Monitoring

- `BATTERY_SERVICE_ENABLED`
- `BATTERY_ADC_CHANNEL`
- Divider: `BATTERY_DIVIDER_R_TOP`, `BATTERY_DIVIDER_R_BOTTOM`
- Range for percentage: `BATTERY_MAX_MV`, `BATTERY_MIN_MV`
- Protection thresholds:
  - `BATTERY_WARNING_MV`
  - `BATTERY_EXPOSURE_LOCK_MV`
  - `BATTERY_CRITICAL_MV`
- `BATTERY_HYSTERESIS_MV`
- `BATTERY_CRITICAL_SLEEP_DELAY_MS`
- `BATTERY_FILTER_SAMPLES_COUNT`
- Debug bypass:
  - `IGNORE_BATTERY_LOW`
  - `IGNORE_BATTERY_CRITICAL`

## Flash Storage Mapping

- `SETTINGS_FLASH_PAGE_SIZE`, `PRESETS_FLASH_PAGE_SIZE`
- `SETTINGS_FLASH_ADDR`
- `PRESETS_FLASH_ADDR`
- `PRESETS_MAX_COUNT`

With built-in overlap and range checks.

## 3. Battery Logic Mapping

Actual code behavior:

1. Battery is measured periodically in `app_controller`.
2. Exposure start is blocked when battery is below low threshold (with hysteresis), unless `IGNORE_BATTERY_LOW == 1`.
3. Critical battery triggers warning screen and delayed sleep (`BATTERY_CRITICAL_SLEEP_DELAY_MS`), unless `IGNORE_BATTERY_CRITICAL == 1`.
4. After wake, battery is measured again; if still critical, device goes back through the same flow.

## 4. Settings and Persistence

`settings_service` stores `settings_t` with CRC.

- Valid stored settings: loaded as-is.
- Invalid settings: initialized from `SETTINGS_DEFAULT_*` and then saved.

If you want new defaults to take effect on an already-used device, erase/reset settings page.

## 5. Presets and Persistence

Presets are independent from settings:

- Separate Flash page (`PRESETS_FLASH_ADDR`)
- Append-only records with active/tombstone flags
- Replay on boot to rebuild active list
- GC only when needed

## 6. Recommended Dev vs Production Flags

Development:

- `DEBUG = 1`
- `IGNORE_BATTERY_LOW = 1` (optional)
- `IGNORE_BATTERY_CRITICAL = 1` (optional)

Production:

- `DEBUG = 0`
- `IGNORE_BATTERY_LOW = 0`
- `IGNORE_BATTERY_CRITICAL = 0`

## 7. Change Procedure

When changing `config.h`:

1. Rebuild firmware.
2. Flash device.
3. If change affects persisted defaults, erase/reset settings page (or full chip erase) to apply defaults on next boot.
