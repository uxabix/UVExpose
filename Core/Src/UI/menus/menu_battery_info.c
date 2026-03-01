#include "UI/menus/menu_battery_info.h"
#include "UI/ui_manager.h"
#include "UI/menus/menu_settings.h"
#include "Display/display.h"
#include "Services/battery_service.h"
#include "config.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>

#define ITEM_COUNT 5
#define UPDATE_PERIOD_MS 400u

static char items[ITEM_COUNT][16] = {
    "Volt:0mV",
    "Charge:0%",
    "U:0 L:0",
    "Low:0 C:0",
    "Exp:OK"
};

static uint32_t last_update_ms = 0u;

static void update_values(void)
{
    BatteryService_Measure();

    uint16_t voltage_mv = BatteryService_GetVoltageMv();
    uint8_t percentage = BatteryService_GetPercentage();
    uint8_t exposure_allowed = BatteryService_IsExposureAllowed();

    snprintf(items[0], sizeof(items[0]), "Volt:%umV", (unsigned)voltage_mv);
    snprintf(items[1], sizeof(items[1]), "Charge:%u%%", (unsigned)percentage);
    snprintf(items[2], sizeof(items[2]), "U:%u L:%u", (unsigned)BATTERY_MAX_MV, (unsigned)BATTERY_MIN_MV);
    snprintf(items[3], sizeof(items[3]), "Low:%u C:%u", (unsigned)BATTERY_EXPOSURE_LOCK_MV, (unsigned)BATTERY_CRITICAL_MV);
    snprintf(items[4], sizeof(items[4]), "Exp:%s", exposure_allowed ? "OK" : "LOCK");
}

static void on_enter(void)
{
    last_update_ms = 0u;
    update_values();
}

static void on_event(ui_event_t event)
{
    if (event == UI_EVENT_LONG_CLICK || event == UI_EVENT_CLICK) {
        UI_SetMenu(&menu_settings);
    }
}

static void on_render(void)
{
    uint32_t now = HAL_GetTick();
    if ((now - last_update_ms) >= UPDATE_PERIOD_MS) {
        update_values();
        last_update_ms = now;
    }

    display_menu(items, ITEM_COUNT, 255u, 0u);
}

const menu_t menu_battery_info = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
