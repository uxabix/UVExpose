#include "Safety/safety_manager.h"
#include "config.h"
#include "main.h"
#include "Services/exposure_service.h"
#include "Services/buzzer.h"
#include "UI/menus/menu_settings.h"
#include "Services/adc_service.h" // Use the new ADC service

static uint8_t lid_open = 0;
static uint8_t paused_by_safety = 0;
static uint16_t hall_last_raw = 0;
static uint16_t hall_last_mv = 0;

static uint8_t _resolve_lid_state_with_hysteresis(uint16_t mv, uint16_t open_thr, uint16_t close_thr, uint8_t current_state)
{
    // Normal polarity: OPEN threshold is above CLOSE threshold.
    if (open_thr > close_thr) {
        if (mv >= open_thr) {
            return 1u;
        }
        if (mv <= close_thr) {
            return 0u;
        }
        return current_state;
    }

    // Inverted polarity: OPEN threshold is below CLOSE threshold.
    if (open_thr < close_thr) {
        if (mv <= open_thr) {
            return 1u;
        }
        if (mv >= close_thr) {
            return 0u;
        }
        return current_state;
    }

    // Degenerate case (no hysteresis window).
    return (mv >= open_thr) ? 1u : 0u;
}

// Helper function to read and convert Hall sensor value
static uint16_t _get_hall_sensor_mv() {
#if LID_HALL_USE_ADC && defined(HAL_ADC_MODULE_ENABLED)
    hall_last_raw = AdcService_ReadChannel(LID_HALL_ADC_CHANNEL);
    uint16_t mv = AdcService_RawToMv(hall_last_raw);
    if (LID_HALL_VOLTAGE_DIVIDER_PRESENT) {
        // Note: This calculation assumes DIVIDER_R_TOP and DIVIDER_R_BOTTOM are for the Hall sensor
        mv = ((uint64_t)mv * (DIVIDER_R_TOP + DIVIDER_R_BOTTOM)) / DIVIDER_R_BOTTOM;
    }
    hall_last_mv = mv;
    return hall_last_mv;
#else
    // Digital reading fallback. Note: This assumes active-high logic from config.
    hall_last_mv = (HAL_GPIO_ReadPin(Hall_GPIO_Port, Hall_Pin) == LID_HALL_ACTIVE_LEVEL) ? 3300 : 0;
    hall_last_raw = (hall_last_mv > 0) ? ADC_RESOLUTION_MAX : 0;
    return hall_last_mv;
#endif
}


void Safety_Init(void)
{
    // Read initial state
    uint16_t mv = _get_hall_sensor_mv();

    // Determine initial logical state using thresholds from settings
    uint16_t open_thr = menu_settings_get_lid_open_threshold_mv();
    uint16_t close_thr = menu_settings_get_lid_close_threshold_mv();
    lid_open = _resolve_lid_state_with_hysteresis(mv, open_thr, close_thr, 0u);
    
    paused_by_safety = 0;
}

void Safety_Process(void)
{
    uint16_t mv = _get_hall_sensor_mv();
    uint16_t open_thr = menu_settings_get_lid_open_threshold_mv();
    uint16_t close_thr = menu_settings_get_lid_close_threshold_mv();
    uint8_t previous_state = lid_open;

    // If protection is disabled in settings, just update the internal state and exit.
    if (!menu_settings_get_open_lid_protection()) {
        if (paused_by_safety) {
            Exposure_Resume();
            paused_by_safety = 0;
            Buzzer_Stop();
        }
        // Update internal state but do nothing else
        if (mv >= open_thr) {
            lid_open = 1;
        } else if (mv <= close_thr) {
            lid_open = 0;
        }
        return;
    }

    // Hysteresis with automatic polarity handling:
    // - open_thr > close_thr: normal polarity
    // - open_thr < close_thr: inverted polarity
    lid_open = _resolve_lid_state_with_hysteresis(mv, open_thr, close_thr, lid_open);
    
    // If state has not changed, do nothing.
    if (lid_open == previous_state) {
        return;
    }

    // State has changed, act on it.
    if (lid_open)
    {
        // Lid just opened: pause exposure (if running) and start continuous beep
        if (Exposure_IsRunning()) {
            Exposure_Pause();
            paused_by_safety = 1;
        } else {
            paused_by_safety = 0;
        }
        Buzzer_SetMode(BUZZER_MODE_BEEP_UNTIL_STOP);
    }
    else
    {
        // Lid just closed: resume exposure if we paused it and stop buzzer
        if (paused_by_safety) {
            Exposure_Resume();
            paused_by_safety = 0;
        }
        Buzzer_Stop();
    }
}

uint8_t Safety_IsLidOpen(void)
{
    return lid_open;
}

uint8_t Safety_CanSleep(void)
{
    // STOP mode allowed only if:
    // - exposure is not running
    // - lid is closed
    // - no error
    // - timer not running
    if (Exposure_IsRunning()) return 0;
    if (Safety_IsLidOpen()) return 0;
    //if (Exposure_HasError()) return 0;
    //if (Exposure_TimerActive()) return 0;
    return 1;
}

uint16_t Safety_GetHallSensorMv(void)
{
    return hall_last_mv;
}

uint16_t Safety_GetHallSensorRaw(void)
{
    return hall_last_raw;
}
