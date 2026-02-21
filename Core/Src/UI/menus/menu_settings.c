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

#include <stdbool.h>
#include <string.h>

static uint8_t selected_row_index = 0;
#define ITEM_COUNT 8

static bool burn_in_protection = true;
static bool open_lid_protection = true;

// Buzzer settings
static uint8_t beep_count = 1;           // Number of beeps (1-10)
static uint16_t beep_duration = 300;     // Duration of each beep (50-1000ms)
static uint16_t beep_period = 200;       // Pause between beeps (50-2000ms)

#define SLEEP_OPTION_COUNT 6
static uint8_t sleep_mode = 0;
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
    "Beep dur:300ms",
    "Beep pause:200m",
    "LidOpen:2000mV",
    " Close:1800mV",
};

static void update_display_text()
{
    items[0][0] = burn_in_protection ? '+' : '-';
    items[2][0] = open_lid_protection ? '+' : '-';

    // Update sleep mode text
    if (sleep_mode > 0 && sleep_mode < SLEEP_OPTION_COUNT) {
        strcpy(items[1], "Sleep after ");
        strcat(items[1], sleep_options[sleep_mode]);
    } else {
        strcpy(items[1], "Sleep Off");
    }
    
    // Update buzzer settings
    snprintf(items[3], sizeof(items[3]), "Beep count:%u", beep_count);
    snprintf(items[4], sizeof(items[4]), "Beep dur:%u", beep_duration);
    snprintf(items[5], sizeof(items[5]), "Beep pause:%u", beep_period);

    // Update lid thresholds display
    snprintf(items[6], sizeof(items[6]), "LidOpen:%umV", (unsigned)lid_open_threshold_mv);
    snprintf(items[7], sizeof(items[7]), "  Close:%umV", (unsigned)lid_close_threshold_mv);
}

static void on_enter(void)
{
	selected_row_index = 0;
    update_display_text();
}

static void on_event(ui_event_t event)
{
    switch(event)
    {
        case UI_EVENT_ROTATE_CW:
        	selected_row_index = (selected_row_index + 1) % ITEM_COUNT;
            break;

        case UI_EVENT_ROTATE_CCW:
        	selected_row_index = (selected_row_index + ITEM_COUNT - 1) % ITEM_COUNT;
            break;

        case UI_EVENT_CLICK:
            if(selected_row_index == 0) {
                burn_in_protection = !burn_in_protection;
                update_display_text();
            } else if (selected_row_index == 1) {
                sleep_mode = (sleep_mode + 1) % SLEEP_OPTION_COUNT;
                update_display_text();
            } else if (selected_row_index == 2) {
                open_lid_protection = !open_lid_protection;
                update_display_text();
            } else if (selected_row_index == 3) {
                // Beep count: cycle through 1-10
                beep_count = (beep_count % 10) + 1;
                update_display_text();
            } else if (selected_row_index == 4) {
                // Beep duration: cycle through 50, 100, 150...1000
                beep_duration += 50;
                if (beep_duration > 1000) beep_duration = 50;
                update_display_text();
            } else if (selected_row_index == 5) {
                // Beep period: cycle through 50, 100, 150...2000
                beep_period += 50;
                if (beep_period > 2000) beep_period = 50;
                update_display_text();
            } else if (selected_row_index == 6) {
                // Lid open threshold: increase by 50mV, wrap at ADC_VREF_MV
                lid_open_threshold_mv += 50;
                if (lid_open_threshold_mv > ADC_VREF_MV) lid_open_threshold_mv = 0;
                update_display_text();
            } else if (selected_row_index == 7) {
                // Lid close threshold: increase by 50mV, wrap at ADC_VREF_MV
                lid_close_threshold_mv += 50;
                if (lid_close_threshold_mv > ADC_VREF_MV) lid_close_threshold_mv = 0;
                update_display_text();
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
    display_menu(items, ITEM_COUNT, selected_row_index, selected_row_index);
}

// ========== Getters for buzzer settings ==========
uint8_t menu_settings_get_beep_count(void)
{
    return beep_count;
}

uint16_t menu_settings_get_beep_duration(void)
{
    return beep_duration;
}

uint16_t menu_settings_get_beep_period(void)
{
    return beep_period;
}

uint8_t menu_settings_get_open_lid_protection(void)
{
    return open_lid_protection ? 1 : 0;
}

uint16_t menu_settings_get_lid_open_threshold_mv(void)
{
    return lid_open_threshold_mv;
}

uint16_t menu_settings_get_lid_close_threshold_mv(void)
{
    return lid_close_threshold_mv;
}

void menu_settings_set_lid_open_threshold_mv(uint16_t mv)
{
    lid_open_threshold_mv = mv;
    update_display_text();
}

void menu_settings_set_lid_close_threshold_mv(uint16_t mv)
{
    lid_close_threshold_mv = mv;
    update_display_text();
}

const menu_t menu_settings = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
