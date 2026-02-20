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
static uint8_t time_digit_index = 0; // 0-3: M1 M2 S1 S2
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

// mapping digit index 0-3 (M1 M2 S1 S2) to character position in items[0]
static const uint8_t time_digit_pos[4] = {6, 7, 9, 10};

static uint8_t scroll_offset = 0; // first visible menu row
#define VISIBLE_ROWS 4 // adjust to how many rows fit on screen

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
    scroll_offset = 0;
    update_all_items();
}

static void edit_time(ui_event_t event) {
    switch(event)
    {
        case UI_EVENT_ROTATE_CW:
            switch(time_digit_index)
            {
                case 0: // tens of minutes
                    time_minutes = ((time_minutes / 10 + 1) % 10) * 10 + (time_minutes % 10);
                    break;
                case 1: // units of minutes
                    time_minutes = (time_minutes / 10) * 10 + ((time_minutes % 10 + 1) % 10);
                    break;
                case 2: // tens of seconds
                    time_seconds = ((time_seconds / 10 + 1) % 6) * 10 + (time_seconds % 10);
                    break;
                case 3: // units of seconds
                    time_seconds = (time_seconds / 10) * 10 + ((time_seconds % 10 + 1) % 10);
                    break;
            }
            format_time();
            break;

        case UI_EVENT_ROTATE_CCW:
            switch(time_digit_index)
            {
                case 0: // tens of minutes
                    time_minutes = ((time_minutes / 10 + 9) % 10) * 10 + (time_minutes % 10);
                    break;
                case 1: // units of minutes
                    time_minutes = (time_minutes / 10) * 10 + ((time_minutes % 10 + 9) % 10);
                    break;
                case 2: // tens of seconds
                    time_seconds = ((time_seconds / 10 + 5) % 6) * 10 + (time_seconds % 10);
                    break;
                case 3: // units of seconds
                    time_seconds = (time_seconds / 10) * 10 + ((time_seconds % 10 + 9) % 10);
                    break;
            }
            format_time();
            break;

        case UI_EVENT_CLICK: // move cursor to next digit
            time_digit_index = (time_digit_index + 1) % 4;
            break;

        case UI_EVENT_LONG_CLICK: // exit edit mode
            edit_time_mode = 0;
            break;

        default:
            break;
    }
}

static void on_event(ui_event_t event)
{
    if(edit_time_mode)
    {
        edit_time(event);
        return;
    }

    switch(event)
    {
        case UI_EVENT_ROTATE_CW:
            selected_row_index = (selected_row_index + 1) % ITEM_COUNT;
            if(selected_row_index >= scroll_offset + VISIBLE_ROWS)
                scroll_offset = selected_row_index - VISIBLE_ROWS + 1;
            else if (selected_row_index < scroll_offset)
                scroll_offset = selected_row_index;
            break;

        case UI_EVENT_ROTATE_CCW:
            selected_row_index = (selected_row_index + ITEM_COUNT - 1) % ITEM_COUNT;
            if(selected_row_index < scroll_offset)
                scroll_offset = selected_row_index;
            else if (selected_row_index >= scroll_offset + VISIBLE_ROWS)
                scroll_offset = selected_row_index - VISIBLE_ROWS + 1;
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
                    // Save preset
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
    if(edit_time_mode && selected_row_index == 0)
    {
        uint8_t highlight_pos = time_digit_pos[time_digit_index]; // get correct char index
        display_menu_column(items, ITEM_COUNT, selected_row_index, scroll_offset, highlight_pos);
    }
    else
    {
        display_menu(items, ITEM_COUNT, selected_row_index, scroll_offset);
    }
}

const menu_t menu_expose_options = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};