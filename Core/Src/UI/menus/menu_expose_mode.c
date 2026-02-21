/*
 * menu_time_select.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_expose_mode.h"
#include "UI/ui_manager.h"
#include "UI/menus/menu_main.h"
#include "UI/menus/menu_presets.h"
#include "UI/menus/menu_running.h"
#include "UI/menus/menu_expose_options.h"
#include "Display/display.h"

#include <string.h>

static uint8_t selected_row_index = 0;
#define ITEM_COUNT 3

static char items[ITEM_COUNT][16] = {
    "Presets",
    "Custom",
    "Active until off"
};

static void on_enter(void)
{
	selected_row_index = 0;
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
                // Presets - go to presets menu
                menu_running_set_infinite_mode(0);  // Normal mode
                UI_SetMenu(&menu_presets);
            }
            else if(selected_row_index == 1) {
                // Custom - set infinite mode off and go to expose_options
                menu_running_set_infinite_mode(0);  // Normal mode
                menu_expose_options_set_until_off(0);
                UI_SetMenu(&menu_expose_options);
            }
            else if(selected_row_index == 2) {
                // Active until off - enable infinite mode and start exposure
                menu_running_set_infinite_mode(1);  // Infinite mode - no timer
                UI_SetMenu(&menu_running);
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

const menu_t menu_expose_mode = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
