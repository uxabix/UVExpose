#include "Services/power_manager.h"
#include "Services/settings_service.h"
#include "Safety/safety_manager.h"
#include "Display/display.h"
#include "config.h"
#include "main.h"
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

#if (DEBUG == 1)
#define DEBUG_LED_BLINK_COUNT 3u
#define DEBUG_LED_BLINK_ON_MS 70u
#define DEBUG_LED_BLINK_OFF_MS 70u
#define DEBUG_LED_ACTIVE_LEVEL GPIO_PIN_RESET

static GPIO_PinState debug_led_inactive_level(void)
{
    return (DEBUG_LED_ACTIVE_LEVEL == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
}

static void debug_led_set(uint8_t on)
{
    HAL_GPIO_WritePin(LED_Debug_GPIO_Port, LED_Debug_Pin,
                      on ? DEBUG_LED_ACTIVE_LEVEL : debug_led_inactive_level());
}
#else
static void debug_led_set(uint8_t on)
{
    (void)on;
}
#endif

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
#if (DEBUG == 1)
    // In DEBUG build keep LED on while MCU is in STOP.
    debug_led_set(1u);
#endif

    // Turn off display and peripherals before sleeping
    display_off();
    HAL_SuspendTick();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    HAL_ResumeTick();
    SystemClock_Config();

    // Re-init display
    display_init();
    display_on();

#if (DEBUG == 1)
    // Turn off debug LED as soon as MCU wakes.
    debug_led_set(0u);
#endif

    power_manager_notify_activity(); // Reset activity after wakeup
}

void power_manager_debug_startup_blink(void)
{
#if (DEBUG == 1)
    debug_led_set(0u);
    for (uint8_t i = 0u; i < DEBUG_LED_BLINK_COUNT; ++i) {
        debug_led_set(1u);
        HAL_Delay(DEBUG_LED_BLINK_ON_MS);
        debug_led_set(0u);
        HAL_Delay(DEBUG_LED_BLINK_OFF_MS);
    }
#endif
}
