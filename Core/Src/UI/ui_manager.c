/*
 * ui_manager.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */
#include "UI/ui_manager.h"
#include "UI/menus/menu_main.h"
#include "UI/menus/menu_running.h"
#include "UI/menus/menu_expose_mode.h"
#include "UI/menus/menu_settings.h"
#include "UI/menus/menu_presets.h"
#include "UI/menus/menu_expose_options.h"

#include <stddef.h>

static const menu_t *current_menu = NULL;

void UI_Init(void)
{
    UI_SetMenu(&menu_main);
}

void UI_SetMenu(const menu_t *menu)
{
    current_menu = menu;
    if(current_menu != NULL && current_menu->on_enter != NULL)
    {
        current_menu->on_enter();
    }
}

void UI_HandleEvent(ui_event_t event)
{
    if(current_menu != NULL && current_menu->on_event != NULL)
    {
        current_menu->on_event(event);
    }
}

void UI_Render(void)
{
    if(current_menu != NULL && current_menu->on_render != NULL)
    {
        current_menu->on_render();
    }
}
