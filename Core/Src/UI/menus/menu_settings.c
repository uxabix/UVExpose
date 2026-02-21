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

#include <stdbool.h>
#include <string.h>

static uint8_t selected_row_index = 0;
#define ITEM_COUNT 3

static bool burn_in_protection = true;
static bool open_lid_protection = true;

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


static char items[ITEM_COUNT][16] = {
    "+Burn-in prot.",
    "Sleep after 2m",
    "+Open lid prot."
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

const menu_t menu_settings = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
