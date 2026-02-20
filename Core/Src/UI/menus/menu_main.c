/*
 * menu_main.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_main.h"
#include "UI/ui_manager.h"
#include "Display/display.h"

#include "UI/menus/menu_running.h"
#include "UI/menus/menu_expose_mode.h"
#include "UI/menus/menu_settings.h"
#include "UI/menus/menu_presets.h"


static int index = 0;
#define ITEM_COUNT 3

static char items[ITEM_COUNT][16] = {
    "Start",
    "Settings",
    "Presets"
};

static void on_enter(void)
{
    index = 0;
}

static void on_event(ui_event_t event)
{
    switch(event)
    {
        case UI_EVENT_ROTATE_CW:
            index = (index + 1) % ITEM_COUNT;
            break;

        case UI_EVENT_ROTATE_CCW:
            index = (index + ITEM_COUNT - 1) % ITEM_COUNT;
            break;

        case UI_EVENT_CLICK:
            if(index == 0) UI_SetMenu(&menu_expose_mode);
            if(index == 1) UI_SetMenu(&menu_settings);
            if(index == 2) UI_SetMenu(&menu_presets);
            break;

        // default:
            break;
    }
}

static void on_render(void)
{
    display_menu(&items, ITEM_COUNT, index, index);
}

const menu_t menu_main = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
