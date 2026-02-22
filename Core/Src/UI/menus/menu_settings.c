/*
 * menu_settings.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_settings.h"
#include "UI/ui_manager.h"
#include "Display/display.h"
#include "UI/menus/menu_main.h"

#include "config.h"
#include "Services/settings_service.h"
#include "Helpers/simple_formatters.h"

#include <stdbool.h>
#include <string.h>


// Menu state variables
static uint8_t selected_row_index = 0;
static uint8_t scroll_offset = 0;

#define VISIBLE_ROWS 4
#define ITEM_COUNT 9

#define SLEEP_OPTION_COUNT 6
static const char* sleep_options[] = {
    "Off",
    "2m",
    "5m",
    "10m",
    "15m",
    "30m",
};


// Thresholds for linear Hall sensor (millivolts), initialized from config defaults
static uint16_t lid_open_threshold_mv = LID_HALL_OPEN_THRESHOLD_MV;
static uint16_t lid_close_threshold_mv = LID_HALL_CLOSE_THRESHOLD_MV;

static char items[ITEM_COUNT][16] = {
    "+Burn-in prot.",
    "Sleep after 2m",
    "+Open lid prot.",
    "Beep count:1",
    "BeepDur:300ms",
    "  Pause:200ms",
    "LidOpen:2000mV",
    "  Close:1800mV",
    "Save settings",
};

static void update_display_text()
{
    // Clear all item strings to avoid garbage
    for (int i = 0; i < ITEM_COUNT; ++i) {
        items[i][0] = '\0';
    }

    // Restore static text for Save settings
    strcpy(items[8], "Save settings");

    // Update toggles
    strcpy(items[0], g_settings.burn_in_protection ? "+Burn-in prot." : "-Burn-in prot.");
    if (g_settings.sleep_mode > 0 && g_settings.sleep_mode < SLEEP_OPTION_COUNT) {
        strcpy(items[1], "Sleep after ");
        strcat(items[1], sleep_options[g_settings.sleep_mode]);
    } else {
        strcpy(items[1], "Sleep Off");
    }
    strcpy(items[2], g_settings.open_lid_protection ? "+Open lid prot." : "-Open lid prot.");

    // Update buzzer settings with units (ms)
    simple_utoa(items[3], sizeof(items[3]), g_settings.beep_count, "Beep count:");
    strcat(items[3], "");
    simple_utoa(items[4], sizeof(items[4]), g_settings.beep_duration, "BeepDur:");
    strcat(items[4], "ms");
    simple_utoa(items[5], sizeof(items[5]), g_settings.beep_period, "  Pause:");
    strcat(items[5], "ms");

    // Update lid thresholds display with units (mV)
    simple_utoa(items[6], sizeof(items[6]), g_settings.lid_open_threshold_mv, "LidOpen:");
    strcat(items[6], "mV");
    simple_utoa(items[7], sizeof(items[7]), g_settings.lid_close_threshold_mv, "  Close:");
    strcat(items[7], "mV");
}

static void on_enter(void)
{
    selected_row_index = 0;
    scroll_offset = 0;
    update_display_text();
}

static void on_event(ui_event_t event)
{
    switch(event)
    {
        case UI_EVENT_ROTATE_CW:
            selected_row_index = (selected_row_index + 1) % ITEM_COUNT;

            if(selected_row_index >= scroll_offset + VISIBLE_ROWS)
                scroll_offset = selected_row_index - VISIBLE_ROWS + 1;
            else if(selected_row_index < scroll_offset)
                scroll_offset = selected_row_index;
            break;

        case UI_EVENT_ROTATE_CCW:
            selected_row_index = (selected_row_index + ITEM_COUNT - 1) % ITEM_COUNT;

            if(selected_row_index < scroll_offset)
                scroll_offset = selected_row_index;
            else if(selected_row_index >= scroll_offset + VISIBLE_ROWS)
                scroll_offset = selected_row_index - VISIBLE_ROWS + 1;
            break;

        case UI_EVENT_CLICK:
            if(selected_row_index == 0) {
                g_settings.burn_in_protection = !g_settings.burn_in_protection;
                update_display_text();
            } else if (selected_row_index == 1) {
                g_settings.sleep_mode = (g_settings.sleep_mode + 1) % SLEEP_OPTION_COUNT;
                update_display_text();
            } else if (selected_row_index == 2) {
                g_settings.open_lid_protection = !g_settings.open_lid_protection;
                update_display_text();
            } else if (selected_row_index == 3) {
                // Beep count: cycle through 1-10
                g_settings.beep_count = (g_settings.beep_count % 10) + 1;
                update_display_text();
            } else if (selected_row_index == 4) {
                // Beep duration: cycle through 100, 200, 300...3000
                g_settings.beep_duration += 100;
                if (g_settings.beep_duration > 3000) g_settings.beep_duration = 100;
                update_display_text();
            } else if (selected_row_index == 5) {
                // Beep period: cycle through 50, 100, 150...5000
                g_settings.beep_period += 100;
                if (g_settings.beep_period > 5000) g_settings.beep_period = 100;
                update_display_text();
            } else if (selected_row_index == 6) {
                // Lid open threshold: increase by 50mV, wrap at ADC_VREF_MV
                g_settings.lid_open_threshold_mv += 50;
                if (g_settings.lid_open_threshold_mv > ADC_VREF_MV) g_settings.lid_open_threshold_mv = 0;
                update_display_text();
            } else if (selected_row_index == 7) {
                // Lid close threshold: increase by 50mV, wrap at ADC_VREF_MV
                g_settings.lid_close_threshold_mv += 50;
                if (g_settings.lid_close_threshold_mv > ADC_VREF_MV) g_settings.lid_close_threshold_mv = 0;
                update_display_text();
            } else if (selected_row_index == 8) {
                // Save settings
                Settings_Save(&g_settings);
                UI_SetMenu(&menu_main);
            }
            break;
        
        case UI_EVENT_LONG_CLICK:
            UI_SetMenu(&menu_main);
            break;

        default:
            break;
    }
}

static void on_render(void)
{
    display_menu(items, ITEM_COUNT, selected_row_index, scroll_offset);
}

// ========== Getters for buzzer settings ==========
uint8_t menu_settings_get_beep_count(void)
{
    return g_settings.beep_count;
}

uint16_t menu_settings_get_beep_duration(void)
{
    return g_settings.beep_duration;
}

uint16_t menu_settings_get_beep_period(void)
{
    return g_settings.beep_period;
}

uint8_t menu_settings_get_open_lid_protection(void)
{
    return g_settings.open_lid_protection ? 1 : 0;
}

uint16_t menu_settings_get_lid_open_threshold_mv(void)
{
    return g_settings.lid_open_threshold_mv;
}

uint16_t menu_settings_get_lid_close_threshold_mv(void)
{
    return g_settings.lid_close_threshold_mv;
}

void menu_settings_set_lid_open_threshold_mv(uint16_t mv)
{
    g_settings.lid_open_threshold_mv = mv;
    update_display_text();
}

void menu_settings_set_lid_close_threshold_mv(uint16_t mv)
{
    g_settings.lid_close_threshold_mv = mv;
    update_display_text();
}

const menu_t menu_settings = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
