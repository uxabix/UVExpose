#include "UI/menus/menu_hall_info.h"
#include "UI/ui_manager.h"
#include "UI/menus/menu_settings.h"
#include "Display/display.h"
#include "Safety/safety_manager.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>

#define ITEM_COUNT 4
#define UPDATE_PERIOD_MS 250u

static char items[ITEM_COUNT][16] = {
    "Raw:0",
    "Hall:0mV",
    "O:0 C:0",
    "Lid:CLOSED"
};

static uint32_t last_update_ms = 0u;

static void update_values(void)
{
    uint16_t hall_raw = Safety_GetHallSensorRaw();
    uint16_t hall_mv = Safety_GetHallSensorMv();
    uint16_t open_thr = menu_settings_get_lid_open_threshold_mv();
    uint16_t close_thr = menu_settings_get_lid_close_threshold_mv();
    uint8_t lid_open = Safety_IsLidOpen();

    snprintf(items[0], sizeof(items[0]), "Raw:%u", (unsigned)hall_raw);
    snprintf(items[1], sizeof(items[1]), "Hall:%umV", (unsigned)hall_mv);
    snprintf(items[2], sizeof(items[2]), "O:%u C:%u", (unsigned)open_thr, (unsigned)close_thr);
    snprintf(items[3], sizeof(items[3]), "Lid:%s", lid_open ? "OPEN" : "CLOSED");
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

const menu_t menu_hall_info = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
