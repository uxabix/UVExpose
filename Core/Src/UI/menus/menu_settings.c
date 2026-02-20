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
#define ITEM_COUNT 1

static bool burn_in_protection = true;

static char items[ITEM_COUNT][16] = {
    "Burn-in prot."
};

static void update_display_text()
{
    // This is not very efficient, but simple.
    strcpy(items[0], "Burn-in: ");
    strcat(items[0], burn_in_protection ? "On" : "Off");
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
