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
#include "UI/menus/menu_running.h"
#include "UI/menus/menu_expose_options.h"
#include "Services/presets_service.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

typedef enum {
    PRESETS_UI_STATE_LIST = 0,
    PRESETS_UI_STATE_ACTION
} presets_ui_state_t;

typedef enum {
    PRESET_ACTION_START = 0,
    PRESET_ACTION_DELETE
} preset_action_t;

static uint8_t selected_row_index = 0;
static uint8_t scroll_offset = 0;
static uint8_t preset_count = 0;
static presets_ui_state_t ui_state = PRESETS_UI_STATE_LIST;
static preset_action_t selected_action = PRESET_ACTION_START;

#define VISIBLE_ROWS 4
#define EMPTY_ITEM_COUNT 1
#define ACTION_START_COLUMN 11
#define ACTION_DELETE_COLUMN 12

static char empty_items[EMPTY_ITEM_COUNT][16] = {
    "No presets"
};

static char preset_items[PRESETS_MAX_COUNT][16];

static char buzzer_mode_char(uint8_t mode)
{
    switch (mode) {
        case 0: return 's'; // silent
        case 1: return 'o'; // single beep
        case 2: return 'l'; // multiple beeps
        default: return 's';
    }
}

static void clamp_scroll_to_selected(void)
{
    if (preset_count == 0u) {
        scroll_offset = 0u;
        selected_row_index = 0u;
        return;
    }

    if (selected_row_index >= preset_count) {
        selected_row_index = (uint8_t)(preset_count - 1u);
    }

    if (selected_row_index < scroll_offset) {
        scroll_offset = selected_row_index;
    } else if (selected_row_index >= (uint8_t)(scroll_offset + VISIBLE_ROWS)) {
        scroll_offset = (uint8_t)(selected_row_index - VISIBLE_ROWS + 1u);
    }
}

static void refresh_presets_list(void)
{
    preset_count = Presets_Count();

    if (preset_count > PRESETS_MAX_COUNT) {
        preset_count = PRESETS_MAX_COUNT;
    }

    for (uint8_t i = 0u; i < preset_count; ++i) {
        preset_t preset;
        if (Presets_Get(i, &preset) != PRESETS_STATUS_OK) {
            strncpy(preset_items[i], "--.--:--s- >x", sizeof(preset_items[i]));
            preset_items[i][sizeof(preset_items[i]) - 1u] = '\0';
            continue;
        }

        const char buzzer_char = buzzer_mode_char(preset.buzzer_mode);
        const char expose_char = (preset.until_off != 0u) ? 'U' : 'a';
        snprintf(preset_items[i], sizeof(preset_items[i]), "%02u.%02u:%02u%c%c >x",
                 (unsigned)(i + 1u),
                 (unsigned)preset.minutes,
                 (unsigned)preset.seconds,
                 buzzer_char,
                 expose_char);
    }

    clamp_scroll_to_selected();
}

static void start_selected_preset(void)
{
    if (selected_row_index >= preset_count) {
        return;
    }

    preset_t preset;
    if (Presets_Get(selected_row_index, &preset) != PRESETS_STATUS_OK) {
        return;
    }

    const uint32_t total_seconds = ((uint32_t)preset.minutes * 60u) + (uint32_t)preset.seconds;
    menu_expose_options_set_time_ms(total_seconds * 1000u);
    menu_expose_options_set_beep_mode(preset.buzzer_mode);
    menu_expose_options_set_until_off(preset.until_off);

    menu_running_set_infinite_mode(0u);
    UI_SetMenu(&menu_running);
}

static void delete_selected_preset(void)
{
    if (selected_row_index >= preset_count) {
        return;
    }

    (void)Presets_Delete(selected_row_index);
    ui_state = PRESETS_UI_STATE_LIST;
    selected_action = PRESET_ACTION_START;
    refresh_presets_list();
}

static void on_enter(void)
{
	selected_row_index = 0u;
    scroll_offset = 0u;
    ui_state = PRESETS_UI_STATE_LIST;
    selected_action = PRESET_ACTION_START;
    refresh_presets_list();
}

static void on_event(ui_event_t event)
{
    if (preset_count == 0u) {
        if (event == UI_EVENT_LONG_CLICK) {
            UI_SetMenu(&menu_main);
        }
        return;
    }

    if (ui_state == PRESETS_UI_STATE_LIST) {
        switch(event)
        {
            case UI_EVENT_ROTATE_CW:
                selected_row_index = (uint8_t)((selected_row_index + 1u) % preset_count);
                clamp_scroll_to_selected();
                break;

            case UI_EVENT_ROTATE_CCW:
                selected_row_index = (uint8_t)((selected_row_index + preset_count - 1u) % preset_count);
                clamp_scroll_to_selected();
                break;

            case UI_EVENT_CLICK:
                ui_state = PRESETS_UI_STATE_ACTION;
                selected_action = PRESET_ACTION_START;
                break;

            case UI_EVENT_LONG_CLICK:
                UI_SetMenu(&menu_main);
                break;

            default:
                break;
        }
        return;
    }

    switch(event)
    {
        case UI_EVENT_ROTATE_CW:
        case UI_EVENT_ROTATE_CCW:
            selected_action = (selected_action == PRESET_ACTION_START) ? PRESET_ACTION_DELETE : PRESET_ACTION_START;
            break;

        case UI_EVENT_CLICK:
            if (selected_action == PRESET_ACTION_START) {
                start_selected_preset();
            } else {
                delete_selected_preset();
            }
            break;

        case UI_EVENT_LONG_CLICK:
            ui_state = PRESETS_UI_STATE_LIST;
            break;

        default:
            break;
    }
}

static void on_render(void)
{
    if (preset_count == 0u) {
        display_menu(empty_items, EMPTY_ITEM_COUNT, 0u, 0u);
        return;
    }

    if (ui_state == PRESETS_UI_STATE_ACTION) {
        uint8_t selected_column = (selected_action == PRESET_ACTION_START) ? ACTION_START_COLUMN : ACTION_DELETE_COLUMN;
        display_menu_column(preset_items, preset_count, selected_row_index, scroll_offset, selected_column);
    } else {
        display_menu(preset_items, preset_count, selected_row_index, scroll_offset);
    }
}

const menu_t menu_presets = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
