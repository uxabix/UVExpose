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
#include "Helpers/simple_formatters.h"
#include "Services/presets_service.h"

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
    // Always prefix with 'Time: '
    char time_buf[8];
    simple_time_format(time_buf, sizeof(time_buf), time_minutes, time_seconds);
    snprintf(items[0], sizeof(items[0]), "Time: %s", time_buf);
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

// ========== Getters for exposure settings ==========
uint32_t menu_expose_options_get_time_ms(void)
{
    return (uint32_t)(time_minutes * 60 + time_seconds) * 1000;
}

uint8_t menu_expose_options_get_beep_mode(void)
{
    return beep_mode;
}

uint8_t menu_expose_options_get_until_off(void)
{
    return until_off_mode;
}

// ========== Setter for until_off mode ==========
void menu_expose_options_set_until_off(uint8_t enabled)
{
    until_off_mode = enabled;
    update_until_off_item();
}

// ========== Setter for beep mode ==========
void menu_expose_options_set_beep_mode(uint8_t mode)
{
    beep_mode = (mode > 2) ? 0 : mode;
    update_beep_items();
}

// ========== Setter for time ==========
void menu_expose_options_set_time_ms(uint32_t time_ms)
{
    uint32_t total_seconds = time_ms / 1000u;
    time_minutes = total_seconds / 60;
    time_seconds = total_seconds % 60;
    
    // Clamp to max value (99:59)
    if (time_minutes > 99) time_minutes = 99;
    
    format_time();
}

// ========== Debug: Reset to defaults ==========
void menu_expose_options_reset_to_defaults(void)
{
    time_minutes = 15;
    time_seconds = 0;
    beep_mode = 0;
    until_off_mode = 0;
    update_all_items();
}


// ---------- menu ----------

static void on_enter(void)
{
    selected_row_index = 0;
    edit_time_mode = 0;
    time_digit_index = 0;  // Reset digit selection for time edit
    scroll_offset = 0;
    // Don't reset time/beep/until_off - they should persist across visits
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
                {
                    // Save preset
                    preset_t preset;
                    uint32_t total_seconds = menu_expose_options_get_time_ms() / 1000u;
                    if (total_seconds > 5999u) {
                        total_seconds = 5999u;
                    }

                    preset.minutes = (uint8_t)(total_seconds / 60u);
                    preset.seconds = (uint8_t)(total_seconds % 60u);
                    preset.buzzer_mode = beep_mode;
                    preset.until_off = (uint8_t)(until_off_mode ? 1u : 0u);
                    (void)Presets_Add(&preset);
                    break;
                }

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
        // Highlight correct digit in 'Time: MM:SS'
        uint8_t highlight_pos = time_digit_pos[time_digit_index];
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
