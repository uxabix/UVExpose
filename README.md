# UVExpose

UVExpose is a portable UV exposure controller firmware for STM32F103C6.
The project uses a modular architecture with persistent settings, preset storage in Flash, battery protection logic, and an encoder-driven OLED UI.

## Navigation

- [Features](#features)
- [Architecture](#architecture)
- [Schematics](#schematics)
- [Battery Policy (Current Behavior)](#battery-policy-current-behavior)
- [Preset Storage](#preset-storage)
- [Settings Storage](#settings-storage)
- [Debug LED](#debug-led)
- [Encoder](#encoder)
- [Display Burn-in Protection](#display-burn-in-protection)
- [Build and Flash](#build-and-flash)
- [Documentation](#documentation)
- [License](#license)

## Features

- Exposure timer with finite and "until off" modes
- Presets: save, start, delete, deduplication
- Persistent user settings in internal Flash
- Hall sensor lid protection with hysteresis and auto polarity handling
- Battery monitoring with warning/low/critical behavior
- Inactivity sleep mode with wake-up by user input
- OLED menu UI with short/long button press support
- Configurable encoder direction inversion

## Architecture

- `App`: high-level process loop and FSM glue (`app_controller`)
- `Services`: exposure, presets, settings, battery, power, buzzer, ADC, timers
- `Safety`: lid protection logic (`safety_manager`)
- `UI`: menu system and screens
- `Display`: SSD1306 rendering helpers
- `Drivers`: encoder and low-level device drivers

## Schematics

![UVExpose schematic](Demonstration/UV_Expose.svg)

Schematic and hardware files:

- [Demonstration/UV_Expose.svg](Demonstration/UV_Expose.svg)
- [Demonstration/UV_Expose.pdf](Demonstration/UV_Expose.pdf)
- [Schematics/UV_Expose/UV_Expose.kicad_sch](Schematics/UV_Expose/UV_Expose.kicad_sch)
- [Schematics/UV_Expose/UV_Expose.kicad_pcb](Schematics/UV_Expose/UV_Expose.kicad_pcb)
- [Schematics/UV_Expose/UV_Expose.kicad_pro](Schematics/UV_Expose/UV_Expose.kicad_pro)
- [Schematics/UV_Expose/UV_Expose.svg](Schematics/UV_Expose/UV_Expose.svg)
- [Schematics/UV_Expose/UV_Expose.pdf](Schematics/UV_Expose/UV_Expose.pdf)

## Battery Policy (Current Behavior)

Battery thresholds are configured in `Core/Inc/config.h`.

- `BATTERY_WARNING_MV`: warning level
- `BATTERY_EXPOSURE_LOCK_MV`: below this level exposure start is blocked
- `BATTERY_CRITICAL_MV`: critical level for forced sleep
- `BATTERY_HYSTERESIS_MV`: hysteresis for lock/critical recovery
- `BATTERY_CRITICAL_SLEEP_DELAY_MS`: warning delay before entering sleep

Runtime behavior:

1. If voltage is below low threshold, starting exposure is blocked (with hysteresis).
2. If voltage reaches critical threshold, exposure is stopped and the UI shows `Battery critical`.
3. After delay (`BATTERY_CRITICAL_SLEEP_DELAY_MS`), device enters STOP sleep.
4. On wake, battery is checked again; if still critical, the cycle repeats.

Debug bypass flags:

- `IGNORE_BATTERY_LOW`
- `IGNORE_BATTERY_CRITICAL`

Set both to `0` for normal production behavior.

## Preset Storage

Presets are stored on a dedicated Flash page via append-only journal records.

- Identity fields: `minutes`, `seconds`, `buzzer_mode`, `until_off`
- Duplicate presets are not saved
- Delete appends a tombstone record (`active=0`)
- Page erase happens only during GC when needed
- Active presets are rebuilt into RAM by replay at startup

Storage layout:

- `PRESETS_FLASH_ADDR`: presets page
- `SETTINGS_FLASH_ADDR`: settings page
- `PRESETS_MAX_COUNT`: logical UI/RAM limit

## Settings Storage

`settings_service` stores one `settings_t` structure in its own Flash page.

- Save operation: erase page + rewrite struct
- Load operation: CRC validation
- If settings are invalid, defaults from `SETTINGS_DEFAULT_*` macros are used

Important note:

- `SETTINGS_DEFAULT_*` values are applied only when stored settings are invalid/empty.
- If you already have valid settings in Flash, changing defaults alone will not override them.

## Debug LED

Debug LED behavior is enabled only when `DEBUG == 1` in `config.h`.

- Startup: blink 3 times
- Entering STOP: LED ON
- Wake from STOP: LED OFF

With `DEBUG == 0`, this behavior is disabled.

## Encoder

- Button debounce is configured by `ENCODER_BUTTON_DEBOUNCE_MS`
- Rotation direction can be inverted by `ENCODER_INVERT_DIRECTION`:
  - `0`: normal
  - `1`: inverted

## Display Burn-in Protection

When burn-in protection setting is enabled (`g_settings.burn_in_protection`), UI elements can move slightly.

Menu movement is configurable in `config.h`:

- `SCREEN_MENU_BURNIN_SHIFT_MAX_X`
- `SCREEN_MENU_BURNIN_SHIFT_MAX_Y`
- `SCREEN_MENU_BURNIN_STEP_MS`

## Build and Flash

1. Open project in STM32CubeIDE
2. Regenerate code from `.ioc` if needed
3. Build
4. Flash via ST-Link

If linker/data page mapping was changed, do full erase before first run to avoid stale settings/presets conflicts.

## Documentation

- [CONFIG_GUIDE.md](CONFIG_GUIDE.md): detailed `config.h` reference
- [Core/Inc/config.h](Core/Inc/config.h): main firmware configuration
- [UVExpose.ioc](UVExpose.ioc): STM32CubeMX project file
- Source comments: implementation details

## License

MIT License. See `LICENSE`.
