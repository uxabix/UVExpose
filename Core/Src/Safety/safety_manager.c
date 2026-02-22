#include "Safety/safety_manager.h"
#include "config.h"
#include "main.h"
#include "Services/exposure_service.h"
#include "Services/buzzer.h"
#include "UI/menus/menu_settings.h"
#include "stm32f1xx_hal.h"
#if LID_HALL_USE_ADC && defined(HAL_ADC_MODULE_ENABLED)
extern ADC_HandleTypeDef hadc1; // Ensure ADC handle exists if ADC is enabled
#endif

static uint8_t lid_open = 0;
static uint8_t paused_by_safety = 0;

void Safety_Init(void)
{
    // Read initial state (analog or digital)
    uint32_t mv = 0;
    #if LID_HALL_USE_ADC && defined(HAL_ADC_MODULE_ENABLED)
    // perform a single ADC read
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        uint32_t raw = HAL_ADC_GetValue(&hadc1);
        mv = (raw * ADC_VREF_MV) / ADC_RESOLUTION_MAX;
    }
    HAL_ADC_Stop(&hadc1);
    #else
    mv = (HAL_GPIO_ReadPin(Hall_GPIO_Port, Hall_Pin) == GPIO_PIN_SET) ? ADC_VREF_MV : 0;
    #endif

    // Determine initial logical state using thresholds from settings
    uint16_t open_thr = menu_settings_get_lid_open_threshold_mv();
    uint16_t close_thr = menu_settings_get_lid_close_threshold_mv();
    if (mv >= open_thr) lid_open = 1;
    else if (mv <= close_thr) lid_open = 0;
    paused_by_safety = 0;
}

void Safety_Process(void)
{
    // If protection disabled in settings, ensure we restore state if we paused
    if (!menu_settings_get_open_lid_protection()) {
        if (paused_by_safety) {
            Exposure_Resume();
            paused_by_safety = 0;
            Buzzer_Stop();
        }
        // update internal state but do nothing else (use analog if available)
        #if LID_HALL_USE_ADC && defined(HAL_ADC_MODULE_ENABLED)
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            uint32_t raw = HAL_ADC_GetValue(&hadc1);
            uint32_t mv = (raw * ADC_VREF_MV) / ADC_RESOLUTION_MAX;
            if (LID_HALL_VOLTAGE_DIVIDER_PRESENT) {
                mv = (mv * (DIVIDER_R_TOP + DIVIDER_R_BOTTOM)) / DIVIDER_R_BOTTOM;
            }
            uint16_t open_thr = menu_settings_get_lid_open_threshold_mv();
            uint16_t close_thr = menu_settings_get_lid_close_threshold_mv();
            if (mv >= open_thr) lid_open = 1;
            else if (mv <= close_thr) lid_open = 0;
        }
        HAL_ADC_Stop(&hadc1);
        #else
        uint16_t pin = HAL_GPIO_ReadPin(Hall_GPIO_Port, Hall_Pin);
        lid_open = (pin == LID_HALL_ACTIVE_LEVEL) ? 1 : 0;
        #endif
        return;
    }

    // Read current sensor value (millivolts)
    uint32_t mv = 0;
    #if LID_HALL_USE_ADC && defined(HAL_ADC_MODULE_ENABLED)
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        uint32_t raw = HAL_ADC_GetValue(&hadc1);
        mv = (raw * ADC_VREF_MV) / ADC_RESOLUTION_MAX;
        if (LID_HALL_VOLTAGE_DIVIDER_PRESENT) {
            mv = (mv * (DIVIDER_R_TOP + DIVIDER_R_BOTTOM)) / DIVIDER_R_BOTTOM;
        }
    }
    HAL_ADC_Stop(&hadc1);
    #else
    mv = (HAL_GPIO_ReadPin(Hall_GPIO_Port, Hall_Pin) == GPIO_PIN_SET) ? ADC_VREF_MV : 0;
    #endif

    uint16_t open_thr = menu_settings_get_lid_open_threshold_mv();
    uint16_t close_thr = menu_settings_get_lid_close_threshold_mv();

    // Hysteresis: only change state when crossing thresholds
    uint8_t current = lid_open;
    if (mv >= open_thr) current = 1;
    else if (mv <= close_thr) current = 0;
    if (current == lid_open) return;

    lid_open = current;
    if (lid_open)
    {
        // Lid opened: pause exposure (if running) and start continuous beep
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
        // Lid closed: resume exposure if we paused it and stop buzzer
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
    if (Exposure_HasError()) return 0;
    if (Exposure_TimerActive()) return 0;
    return 1;
}
