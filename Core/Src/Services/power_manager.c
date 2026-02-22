#include "Services/power_manager.h"
#include "Services/settings_service.h"
#include "Safety/safety_manager.h"
#include "Display/display.h"
#include <stm32f1xx_hal.h>

static uint32_t last_activity_tick = 0;

static const uint32_t sleep_timeouts_s[] = {
    0,      // Off
    120,    // 2m
    300,    // 5m
    600,    // 10m
    900,    // 15m
    1800    // 30m
};
#define SLEEP_TIMEOUTS_COUNT (sizeof(sleep_timeouts_s) / sizeof(uint32_t))

void power_manager_notify_activity(void)
{
    last_activity_tick = HAL_GetTick();
}

void power_manager_update(void)
{
    if (g_settings.sleep_mode == 0 || g_settings.sleep_mode >= SLEEP_TIMEOUTS_COUNT)
        return;

    uint32_t timeout = sleep_timeouts_s[g_settings.sleep_mode] * 1000;
    uint32_t now = HAL_GetTick();

    if ((now - last_activity_tick) > timeout)
    {
        // Check safety conditions
        if (Safety_CanSleep())
        {
            power_manager_sleep();
        }
    }
}

void power_manager_sleep(void)
{
    // Turn off display and peripherals before sleeping
    display_off();
    HAL_SuspendTick();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    HAL_ResumeTick();
    SystemClock_Config();

    // Re-init display
    display_init();
    display_on();

    power_manager_notify_activity(); // Reset activity after wakeup
}
