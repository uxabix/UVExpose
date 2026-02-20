/*
 * menu_time_select.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_expose_options.h"
#include "UI/ui_manager.h"
#include "UI/menus/menu_main.h"
#include "UI/menus/menu_running.h"
#include "UI/menus/menu_expose_mode.h"
#include "Display/display.h"

#include <string.h>
#include <stdio.h>

static uint8_t selected_row_index = 0;

static uint8_t edit_time_mode = 0;

static uint8_t time_minutes = 15;
static uint8_t time_seconds = 0;

static uint8_t beep_mode = 0;       // 0 silent, 1 single, 2 multiple
static uint8_t until_off_mode = 0;  // 0 off, 1 on

#define ITEM_COUNT 7

static char items[ITEM_COUNT][16] = {
    "Time: 15:00",
    "+Silent",
    "-Single beep",
    "-Multiple beeps",
    "-Until off",
    "Save preset",
    "Start"
};


// ---------- helpers ----------

static void format_time(void)
{
    snprintf(items[0], sizeof(items[0]), "Time: %02u:%02u", time_minutes, time_seconds);
}

static void update_beep_items(void)
{
    items[1][0] = (beep_mode == 0) ? '+' : '-';
    items[2][0] = (beep_mode == 1) ? '+' : '-';
    items[3][0] = (beep_mode == 2) ? '+' : '-';
}

static void update_until_off_item(void)
{
    items[4][0] = until_off_mode ? '+' : '-';
}

static void update_all_items(void)
{
    format_time();
    update_beep_items();
    update_until_off_item();
}

static void change_time(int8_t delta)
{
    int total = time_minutes * 60 + time_seconds;
    total += delta;

    if(total < 0) total = 0;
    if(total > 5999) total = 5999; // 99:59 максимум

    time_minutes = total / 60;
    time_seconds = total % 60;

    format_time();
}


// ---------- menu ----------

static void on_enter(void)
{
    selected_row_index = 0;
    edit_time_mode = 0;
    update_all_items();
}

static void on_event(ui_event_t event)
{
    if(edit_time_mode)
    {
        switch(event)
        {
            case UI_EVENT_ROTATE_CW:
                change_time(+1);
                break;

            case UI_EVENT_ROTATE_CCW:
                change_time(-1);
                break;

            case UI_EVENT_LONG_CLICK:
                edit_time_mode = 0;
                break;

            default:
                break;
        }
        return;
    }

    switch(event)
    {
        case UI_EVENT_ROTATE_CW:
            selected_row_index = (selected_row_index + 1) % ITEM_COUNT;
            break;

        case UI_EVENT_ROTATE_CCW:
            selected_row_index = (selected_row_index + ITEM_COUNT - 1) % ITEM_COUNT;
            break;

        case UI_EVENT_CLICK:
            switch(selected_row_index)
            {
                case 0:
                    edit_time_mode = 1;
                    break;

                case 1:
                    beep_mode = 0;
                    update_beep_items();
                    break;

                case 2:
                    beep_mode = 1;
                    update_beep_items();
                    break;

                case 3:
                    beep_mode = 2;
                    update_beep_items();
                    break;

                case 4:
                    until_off_mode = !until_off_mode;
                    update_until_off_item();
                    break;

                case 5:
                    // Save preset — оставил пустым
                    break;

                case 6:
                    UI_SetMenu(&menu_running);
                    break;
            }
            break;

        case UI_EVENT_LONG_CLICK:
            UI_SetMenu(&menu_expose_mode);
            break;

        default:
            break;
    }
}

static void on_render(void)
{
    display_menu(items, ITEM_COUNT, selected_row_index, selected_row_index);
}

const menu_t menu_expose_options = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};