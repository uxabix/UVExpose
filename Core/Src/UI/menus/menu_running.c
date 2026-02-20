/*
 * menu_running.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_running.h"
#include "UI/ui_manager.h"
#include "Display/display.h"
#include "UI/menus/menu_main.h"

#include <string.h>

static char message[16] = "Running...";

static void on_enter(void)
{
    // This is where the app_controller would start the exposure timer.
}

static void on_event(ui_event_t event)
{
    switch(event)
    {
        case UI_EVENT_LONG_CLICK:
            // This is where the app_controller would stop the exposure.
            UI_SetMenu(&menu_main);
            break;

        default:
            break;
    }
}

static void on_render(void)
{
    // The app_controller should update the display with the remaining time.
    // For now, just show a static message.
    // I will use display_menu to show a single item message.
    char items[1][16];
    strcpy(items[0], message);
    display_menu(items, 1, 0, 0);
}

const menu_t menu_running = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
