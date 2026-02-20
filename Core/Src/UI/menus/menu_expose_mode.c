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
                // TODO: Set mode to "active until off"
                UI_SetMenu(&menu_presets);
            }
            if(selected_row_index == 1) {
                UI_SetMenu(&menu_expose_options);
            }
            if (selected_row_index == 2) {
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
