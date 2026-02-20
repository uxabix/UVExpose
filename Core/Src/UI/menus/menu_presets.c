/*
 * menu_presets.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_presets.h"
#include "UI/ui_manager.h"
#include "Display/display.h"
#include "UI/menus/menu_main.h"

#include <string.h>

static uint8_t selected_row_index = 0;
#define ITEM_COUNT 1

static char items[ITEM_COUNT][16] = {
    "No presets"
};

static void on_enter(void)
{
	selected_row_index = 0;
}

static void on_event(ui_event_t event)
{
    switch(event)
    {
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

const menu_t menu_presets = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
