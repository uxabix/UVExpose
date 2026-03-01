/*
 * display.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#include <stdbool.h>
#include <string.h>

#include "Display/display.h"
#include "Display/ssd1306.h"
#include "Display/fonts.h"
#include "config.h"
#include "Services/settings_service.h"
#include "Helpers/simple_formatters.h"
#include "stm32f1xx_hal.h"

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define MENU_FONT &Font_7x10
#define MENU_COLOR_DEFAULT SSD1306_COLOR_WHITE
#define MENU_TEXT MENU_FONT, MENU_COLOR_DEFAULT
#define MENU_TEXT_HEIGHT SCREEN_MENU_TEXT_HEIGHT
#define MENU_TEXT_WIDTH SCREEN_MENU_TEXT_WIDTH
#define MENU_PADDING SCREEN_MENU_PADDING

uint8_t screen_width = SCREEN_WIDTH;
uint8_t screen_height = SCREEN_HEIGHT;

uint8_t top_bar_height = SCREEN_TOP_BAR_HEIGHT;
uint8_t top_bar_padding = SCREEN_TOP_BAR_PADDING;

uint8_t battery_pos_x = SCREEN_BATTERY_POS_X;
uint8_t battery_pos_y = SCREEN_BATTERY_POS_Y;
const uint8_t battery_protection_step_x = SCREEN_BATTERY_PROTECTION_STEP_X;
const uint8_t battery_protection_step_y = SCREEN_BATTERY_PROTECTION_STEP_Y;
const uint8_t battery_segment_height = SCREEN_BATTERY_SEGMENT_HEIGHT;
const uint8_t battery_segment_width = SCREEN_BATTERY_SEGMENT_WIDTH;

// Display Menu configuration - imported from config.h
const uint8_t HIGHLIGHT_PADDING_X = SCREEN_MENU_HIGHLIGHT_PADDING_X;
const uint8_t HIGHLIGHT_PADDING_Y = SCREEN_MENU_HIGHLIGHT_PADDING_Y;
const uint8_t SCROLLBAR_WIDTH = SCREEN_MENU_SCROLLBAR_WIDTH;
const uint8_t SCROLLBAR_MARGIN = SCREEN_MENU_SCROLLBAR_MARGIN;

// Structure to hold menu layout parameters
typedef struct {
    uint8_t display_area_y_start;
    uint8_t display_area_height;
    uint8_t item_height;
    uint8_t max_visible_items;
    uint8_t actual_visible_items;
    uint8_t effective_screen_width;
    uint16_t max_display_width_for_text;
    uint8_t start_y;
    uint16_t uniform_rect_width;
    uint8_t uniform_rect_x;
    uint8_t clamped_scroll_offset; // The scroll_offset after clamping
} MenuLayout_t;

#define INVALID_COLUMN_INDEX 255

typedef struct {
    int8_t x;
    int8_t y;
    int8_t dx;
    int8_t dy;
    uint8_t initialized;
    uint32_t last_step_ms;
} MenuBurnInState_t;

static MenuBurnInState_t menu_burnin_state = {0};

// Display Timer configuration - imported from config.h
#define TIMER_FONT &Font_16x26
#define TIMER_TEXT_HEIGHT SCREEN_TIMER_TEXT_HEIGHT
#define TIMER_TEXT_WIDTH SCREEN_TIMER_TEXT_WIDTH
#define TIMER_FLOAT_STEP_X SCREEN_TIMER_FLOAT_STEP_X
#define TIMER_FLOAT_STEP_Y SCREEN_TIMER_FLOAT_STEP_Y
#define TIMER_FLOAT_MARGIN SCREEN_TIMER_FLOAT_MARGIN

typedef enum {
    DISPLAY_TEXT_FLOATING,        // Floating within defined area, bouncing off edges
    DISPLAY_TEXT_CENTERED,        // Centered
    DISPLAY_TEXT_FIXED,           // Fixed position
    DISPLAY_TEXT_MARQUEE          // Running text
} DisplayTextMode_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t dx;
    int16_t dy;
    uint8_t initialized;
} DisplayTextState_t;

static DisplayTextState_t timer_state = {0};

void display_init() {
	SSD1306_Init();
	battery_pos_x = top_bar_padding;
	battery_pos_y = top_bar_padding;
}

void display_clear() {
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_UpdateScreen();
}

void display_clear_top_bar() {
	SSD1306_DrawFilledRectangle(0, 0, screen_width, top_bar_height, SSD1306_COLOR_BLACK);
}

void display_clear_main_frame() {
	SSD1306_DrawFilledRectangle(0, top_bar_height - 1, screen_width + 1, screen_height - top_bar_height, SSD1306_COLOR_BLACK);
}

void display_draw_text_advanced(
    const char* text,
    const FontDef_t* font,
    uint8_t char_width,
    uint8_t char_height,
    uint8_t area_min_x,
    uint8_t area_max_x,
    uint8_t area_min_y,
    uint8_t area_max_y,
    DisplayTextMode_t mode,
    DisplayTextState_t* state,
    int16_t fixed_x,
    int16_t fixed_y,
    uint8_t step_x,
    uint8_t step_y
) {
    uint16_t text_width = strlen(text) * char_width;
    uint16_t text_height = char_height;

    switch (mode) {

    case DISPLAY_TEXT_CENTERED:
        state->x = area_min_x + (area_max_x - area_min_x - text_width) / 2;
        state->y = area_min_y + (area_max_y - area_min_y - text_height) / 2;
        break;

    case DISPLAY_TEXT_FIXED:
        state->x = fixed_x;
        state->y = fixed_y;
        break;

    case DISPLAY_TEXT_FLOATING:
        if (!state->initialized) {
            state->x = area_min_x + (area_max_x - area_min_x - text_width) / 2;
            state->y = area_min_y + (area_max_y - area_min_y - text_height) / 2;
            state->dx = step_x;
            state->dy = step_y;
            state->initialized = 1;
        }

        state->x += state->dx;
        state->y += state->dy;

        if (state->x < area_min_x) {
            state->x = area_min_x;
            state->dx = step_x;
        } else if (state->x > area_max_x - text_width) {
            state->x = area_max_x - text_width;
            state->dx = -step_x;
        }

        if (state->y < area_min_y) {
            state->y = area_min_y;
            state->dy = step_y;
        } else if (state->y > area_max_y - text_height) {
            state->y = area_max_y - text_height;
            state->dy = -step_y;
        }
        break;

    case DISPLAY_TEXT_MARQUEE:
        if (!state->initialized) {
            state->x = area_max_x;
            state->y = area_min_y + (area_max_y - area_min_y - text_height) / 2;
            state->dx = -step_x;
            state->initialized = 1;
        }

        state->x += state->dx;

        if (state->x < area_min_x - text_width) {
            state->x = area_max_x;
        }
        break;
    }

    SSD1306_GotoXY(state->x, state->y);
    SSD1306_Puts(text, font, SSD1306_COLOR_WHITE);
}

void display_text_simple(const char* text)
{
    uint8_t text_len = strlen(text);
    
    // Available display area (accounting for margins)
    uint16_t available_width = screen_width - (2 * TIMER_FLOAT_MARGIN);
    uint16_t available_height = screen_height - top_bar_height - (2 * TIMER_FLOAT_MARGIN);
    
    // Font parameters: {char_width, char_height}
    FontSize_t selected_size = FONT_SIZE_SMALL; // Default fallback
    
    // Try largest font first (16x26)
    if (text_len * 16 <= available_width && 26 <= available_height) {
        selected_size = FONT_SIZE_LARGE;
    }
    // Use small font (7x10) as fallback
    else {
        selected_size = FONT_SIZE_SMALL;
    }
    
    display_text_simple_sized(text, selected_size);
}

void display_text_simple_sized(const char* text, FontSize_t font_size)
{
    static DisplayTextState_t text_state = {0};
    
    DisplayTextMode_t mode =
        g_settings.burn_in_protection ? DISPLAY_TEXT_FLOATING
                           : DISPLAY_TEXT_CENTERED;

    // Select font parameters based on size
    const FontDef_t* font;
    uint8_t char_width;
    uint8_t char_height;
    
    switch (font_size) {
        case FONT_SIZE_LARGE:
            font = &Font_16x26;
            char_width = 16;
            char_height = 26;
        case FONT_SIZE_SMALL:
        default:
            font = &Font_7x10;
            char_width = 7;
            char_height = 10;
            break;

    }

    display_clear_main_frame();

    display_draw_text_advanced(
        text,
        font,
        char_width,
        char_height,
        TIMER_FLOAT_MARGIN,
        screen_width - TIMER_FLOAT_MARGIN,
        top_bar_height + TIMER_FLOAT_MARGIN,
        screen_height - TIMER_FLOAT_MARGIN,
        mode,
        &text_state,
        0, 0,
        TIMER_FLOAT_STEP_X,
        TIMER_FLOAT_STEP_Y
    );

    SSD1306_UpdateScreen();
}

void display_timer_remaining(uint32_t remaining_ms)
{
    char text[16];

    if (remaining_ms >= 0xFFFFF000) {
        strcpy(text, "Active");
    } else {
        uint32_t total_seconds = remaining_ms / 1000;
        uint8_t minutes = total_seconds / 60;
        uint8_t seconds = total_seconds % 60;
        simple_time_format(text, sizeof(text), minutes, seconds);
    }

    display_text_simple(text);
}

// Helper function to calculate menu layout parameters
static void display_menu_calculate_layout(uint8_t options_count, uint8_t initial_scroll_offset, MenuLayout_t* layout) {
    layout->display_area_y_start = top_bar_height;
    layout->display_area_height = screen_height - layout->display_area_y_start;
    layout->item_height = MENU_TEXT_HEIGHT + MENU_PADDING;

    // Calculate how many items the screen can fit
    layout->max_visible_items = layout->display_area_height / layout->item_height;
    if (layout->max_visible_items == 0) { // Ensure at least one item can be displayed
        layout->max_visible_items = 1;
    }

    // Determine the actual number of items to display in the current window
    if (options_count <= layout->max_visible_items) {
        layout->actual_visible_items = options_count;
        layout->clamped_scroll_offset = 0; // No need for scroll if everything fits
    } else {
        layout->actual_visible_items = layout->max_visible_items;
        // Clamp scroll offset: not too big
        uint8_t max_possible_scroll_offset = options_count - layout->actual_visible_items;
        layout->clamped_scroll_offset = initial_scroll_offset;
        if (layout->clamped_scroll_offset > max_possible_scroll_offset) {
            layout->clamped_scroll_offset = max_possible_scroll_offset;
        }
        // Clamp scroll offset: >= 0
        if (layout->clamped_scroll_offset < 0) {
            layout->clamped_scroll_offset = 0;
        }
    }

    // Calculate effective screen width considering scroll bar
    layout->effective_screen_width = screen_width;
    if (options_count > layout->max_visible_items) {
        layout->effective_screen_width -= (SCROLLBAR_WIDTH + SCROLLBAR_MARGIN);
    }

    // Maximum displayed text width (without HIGHLIGHT_PADDING_X)
    // Width available for a text inside of a selection rectangle
    layout->max_display_width_for_text = layout->effective_screen_width - (2 * HIGHLIGHT_PADDING_X);
    if (layout->max_display_width_for_text < MENU_TEXT_WIDTH) { // Guarantee minimal width for a char
        layout->max_display_width_for_text = MENU_TEXT_WIDTH;
    }

    // Calculate initial Y-position for the first visible item
    uint16_t menu_content_height = layout->actual_visible_items * layout->item_height;
    if (options_count <= layout->max_visible_items) {
        // If all items fit, vertically center the entire menu in the available area
        layout->start_y = layout->display_area_y_start + (layout->display_area_height - menu_content_height) / 2;
    } else {
        // If scrolling is required, start from the top of the available area
        layout->start_y = layout->display_area_y_start;
    }
}

// Helper function to preprocess visible menu items (truncate and find max width)
static uint16_t display_menu_preprocess_items(char (*options_list)[16], uint8_t options_count, char display_strings[][16 + 1], const MenuLayout_t* layout) {
    uint16_t current_max_item_text_width = 0; // Max text width among visible items (after truncation)

    for (uint8_t i = 0; i < layout->actual_visible_items; i++) {
        uint8_t option_index = layout->clamped_scroll_offset + i;
        char* original_option = options_list[option_index];
        uint16_t original_text_width = strlen(original_option) * MENU_TEXT_WIDTH;

        if (original_text_width > layout->max_display_width_for_text) {
            uint8_t max_chars_to_display = layout->max_display_width_for_text / MENU_TEXT_WIDTH;
            // Ensure max_chars_to_display does not exceed buffer size
            max_chars_to_display = MIN(max_chars_to_display, 16); // 16 is the max string length in sample_menu
            strncpy(display_strings[i], original_option, max_chars_to_display);
            display_strings[i][max_chars_to_display] = '\0';
            current_max_item_text_width = MAX(current_max_item_text_width, layout->max_display_width_for_text);
        } else {
            strcpy(display_strings[i], original_option);
            current_max_item_text_width = MAX(current_max_item_text_width, original_text_width);
        }
    }
    return current_max_item_text_width;
}

// Helper function to draw menu items and highlights
static void display_menu_get_burnin_offset(
    const MenuLayout_t* layout,
    uint8_t uniform_rect_x,
    uint16_t uniform_rect_width,
    int8_t* out_x,
    int8_t* out_y
) {
    *out_x = 0;
    *out_y = 0;

    if (!g_settings.burn_in_protection) {
        menu_burnin_state.initialized = 0;
        return;
    }

    int16_t free_left = uniform_rect_x;
    int16_t free_right = (int16_t)layout->effective_screen_width - ((int16_t)uniform_rect_x + (int16_t)uniform_rect_width);

    uint16_t menu_content_height = (uint16_t)layout->actual_visible_items * (uint16_t)layout->item_height;
    int16_t free_top = (int16_t)layout->start_y - (int16_t)layout->display_area_y_start;
    int16_t free_bottom = (int16_t)layout->display_area_height - (int16_t)menu_content_height - free_top;

    int8_t max_left = (int8_t)MIN((int16_t)SCREEN_MENU_BURNIN_SHIFT_MAX_X, MAX((int16_t)0, free_left));
    int8_t max_right = (int8_t)MIN((int16_t)SCREEN_MENU_BURNIN_SHIFT_MAX_X, MAX((int16_t)0, free_right));
    int8_t max_up = (int8_t)MIN((int16_t)SCREEN_MENU_BURNIN_SHIFT_MAX_Y, MAX((int16_t)0, free_top));
    int8_t max_down = (int8_t)MIN((int16_t)SCREEN_MENU_BURNIN_SHIFT_MAX_Y, MAX((int16_t)0, free_bottom));

    if ((max_left == 0 && max_right == 0) && (max_up == 0 && max_down == 0)) {
        menu_burnin_state.initialized = 0;
        return;
    }

    if (!menu_burnin_state.initialized) {
        menu_burnin_state.x = 0;
        menu_burnin_state.y = 0;
        menu_burnin_state.dx = 1;
        menu_burnin_state.dy = 1;
        menu_burnin_state.last_step_ms = HAL_GetTick();
        menu_burnin_state.initialized = 1;
    }

    if (menu_burnin_state.x > max_right) menu_burnin_state.x = max_right;
    if (menu_burnin_state.x < -max_left) menu_burnin_state.x = -max_left;
    if (menu_burnin_state.y > max_down) menu_burnin_state.y = max_down;
    if (menu_burnin_state.y < -max_up) menu_burnin_state.y = -max_up;

    uint32_t now = HAL_GetTick();
    if ((now - menu_burnin_state.last_step_ms) >= SCREEN_MENU_BURNIN_STEP_MS) {
        menu_burnin_state.last_step_ms = now;

        menu_burnin_state.x += menu_burnin_state.dx;
        menu_burnin_state.y += menu_burnin_state.dy;

        if (menu_burnin_state.x >= max_right) {
            menu_burnin_state.x = max_right;
            menu_burnin_state.dx = -1;
        } else if (menu_burnin_state.x <= -max_left) {
            menu_burnin_state.x = -max_left;
            menu_burnin_state.dx = 1;
        }

        if (menu_burnin_state.y >= max_down) {
            menu_burnin_state.y = max_down;
            menu_burnin_state.dy = -1;
        } else if (menu_burnin_state.y <= -max_up) {
            menu_burnin_state.y = -max_up;
            menu_burnin_state.dy = 1;
        }
    }

    *out_x = menu_burnin_state.x;
    *out_y = menu_burnin_state.y;
}

static void display_menu_draw_items_with_column(
    char display_strings[][16 + 1],
    uint8_t options_count,
    uint8_t selected,
    MenuLayout_t* layout,
    uint16_t current_max_item_text_width,
    uint8_t selected_column_index
) {
    layout->uniform_rect_width = current_max_item_text_width + (2 * HIGHLIGHT_PADDING_X);
    layout->uniform_rect_x = (layout->effective_screen_width - layout->uniform_rect_width) / 2;
    layout->uniform_rect_x = MAX(0, layout->uniform_rect_x);

    int8_t burnin_x = 0;
    int8_t burnin_y = 0;
    display_menu_get_burnin_offset(layout, layout->uniform_rect_x, layout->uniform_rect_width, &burnin_x, &burnin_y);

    uint8_t current_y = (uint8_t)((int16_t)layout->start_y + (int16_t)burnin_y);

    for (uint8_t i = 0; i < layout->actual_visible_items; i++) {
        uint8_t option_index = layout->clamped_scroll_offset + i;
        char* current_option_to_display = display_strings[i];
        uint16_t current_item_text_width = strlen(current_option_to_display) * MENU_TEXT_WIDTH;

        uint8_t text_x = (uint8_t)((int16_t)layout->uniform_rect_x + (int16_t)burnin_x + (int16_t)HIGHLIGHT_PADDING_X +
                                   (int16_t)(current_max_item_text_width - current_item_text_width) / 2);

        if (option_index == selected) {
            uint8_t rect_x = (uint8_t)((int16_t)layout->uniform_rect_x + (int16_t)burnin_x);
            uint8_t rect_y = current_y - HIGHLIGHT_PADDING_Y;
            uint8_t rect_width = layout->uniform_rect_width;
            uint8_t rect_height = MENU_TEXT_HEIGHT + (2 * HIGHLIGHT_PADDING_Y);

            // Highlight full item
            SSD1306_DrawFilledRectangle(rect_x, rect_y, rect_width, rect_height, SSD1306_COLOR_WHITE);

            // If selected_column_index is valid and within the current option text length, highlight that character
            if (selected_column_index != INVALID_COLUMN_INDEX && selected_column_index < strlen(current_option_to_display)) {
                // Draw text before the highlighted character in black
                char tmp[17];
                strncpy(tmp, current_option_to_display, selected_column_index);
                tmp[selected_column_index] = '\0';
                SSD1306_GotoXY(text_x, current_y);
                SSD1306_Puts(tmp, MENU_FONT, SSD1306_COLOR_BLACK);

                // Draw the highlighted character with inverted colors
                uint8_t char_x = text_x + selected_column_index * MENU_TEXT_WIDTH;
                SSD1306_DrawFilledRectangle(char_x, current_y, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, SSD1306_COLOR_BLACK);

                // Draw the highlighted character in white on the black background
                char highlight_char[2] = {current_option_to_display[selected_column_index], '\0'};
                SSD1306_GotoXY(char_x, current_y);
                SSD1306_Puts(highlight_char, MENU_FONT, SSD1306_COLOR_WHITE);

                // Draw text after the highlighted character in black
                uint8_t remaining_len = strlen(current_option_to_display) - (selected_column_index + 1);
                if (remaining_len > 0) {
                    SSD1306_GotoXY(char_x + MENU_TEXT_WIDTH, current_y);
                    SSD1306_Puts(&current_option_to_display[selected_column_index + 1], MENU_FONT, SSD1306_COLOR_BLACK);
                }
            } else {
                // No column highlight, just draw the text in black on the white highlight
                SSD1306_GotoXY(text_x, current_y);
                SSD1306_Puts(current_option_to_display, MENU_FONT, SSD1306_COLOR_BLACK);
            }

        } else {
            // Not selected, draw normally
            SSD1306_GotoXY(text_x, current_y);
            SSD1306_Puts(current_option_to_display, MENU_FONT, MENU_COLOR_DEFAULT);
        }

        current_y += layout->item_height;
    }
}

// Helper function to draw the scrollbar
static void display_menu_draw_scrollbar(uint8_t options_count, const MenuLayout_t* layout) {
    // Draw scrollbar if there are more items than can fit on screen
    if (options_count > layout->max_visible_items) {
        uint8_t scrollbar_x = screen_width - SCROLLBAR_WIDTH - SCROLLBAR_MARGIN;
        uint8_t scrollbar_track_y = layout->display_area_y_start;
        uint8_t scrollbar_track_height = layout->display_area_height;

        // Draw scrollbar track outline
        SSD1306_DrawRectangle(scrollbar_x, scrollbar_track_y, SCROLLBAR_WIDTH, scrollbar_track_height, SSD1306_COLOR_WHITE);

        // Calculate scrollbar thumb size and position
        float thumb_height_ratio = (float)layout->actual_visible_items / options_count;
        uint8_t thumb_height = (uint8_t)(thumb_height_ratio * scrollbar_track_height);

        // Ensure minimum thumb height for visibility
        if (thumb_height < 5 && options_count > layout->actual_visible_items) {
            thumb_height = 5;
        }
        // Ensure thumb does not exceed track height
        if (thumb_height > scrollbar_track_height) {
            thumb_height = scrollbar_track_height;
        }

        // Calculate thumb position
        // scroll_offset range: from 0 to (options_count - actual_visible_items)
        float thumb_pos_ratio = (float)layout->clamped_scroll_offset / (options_count - layout->actual_visible_items);
        uint8_t thumb_y = scrollbar_track_y + (uint8_t)(thumb_pos_ratio * (scrollbar_track_height - thumb_height));

        // Draw filled thumb
        SSD1306_DrawFilledRectangle(scrollbar_x, thumb_y, SCROLLBAR_WIDTH, thumb_height, SSD1306_COLOR_WHITE);
    }
}

// Main menu display function
void display_menu_column(char (*options_list)[16], uint8_t options_count, uint8_t selected, uint8_t scroll_offset, uint8_t selected_column_index) {
    display_clear_main_frame();

    MenuLayout_t layout;
    display_menu_calculate_layout(options_count, scroll_offset, &layout);

    if (layout.max_visible_items == 0) { // If the screen is too small for even one item
        SSD1306_UpdateScreen();
        return;
    }

    // Buffer for truncated strings
    char display_strings[layout.actual_visible_items][16 + 1];
    uint16_t current_max_item_text_width = display_menu_preprocess_items(options_list, options_count, display_strings, &layout);

    display_menu_draw_items_with_column(display_strings, options_count, selected, &layout, current_max_item_text_width, selected_column_index);
    display_menu_draw_scrollbar(options_count, &layout);

    SSD1306_UpdateScreen();
}

// Main menu display function
void display_menu(char (*options_list)[16], uint8_t options_count, uint8_t selected, uint8_t scroll_offset) {
    display_menu_column(options_list, options_count, selected, scroll_offset, INVALID_COLUMN_INDEX);
}

void display_battery(uint8_t percent) { // Percent - number from 0 to 100
	uint8_t current_x = battery_pos_x;
	uint8_t current_y = battery_pos_y;
	uint8_t stage = (uint8_t)percent / 2; // Values from 0 to 50, 41..50 - 5 bars, 30..39 - 4 bars, ...
	for (uint8_t i=10; i < 45; i+=12) {
		if (stage >= i) {
			SSD1306_DrawFilledRectangle(current_x, current_y, battery_segment_width, battery_segment_height, SSD1306_COLOR_WHITE);
		} else {
			SSD1306_DrawRectangle(current_x, current_y, battery_segment_width, battery_segment_height, SSD1306_COLOR_WHITE);
		}
		current_x += battery_segment_width;
	}

	uint8_t small_segment_width = battery_segment_width * 0.75;
	uint8_t small_segment_height = battery_segment_height * 0.75;
	uint8_t small_segment_y = current_y + (battery_segment_height - small_segment_height) / 2;
	if (stage >= 46) {
		SSD1306_DrawFilledRectangle(current_x, small_segment_y, small_segment_width, small_segment_height, SSD1306_COLOR_WHITE);
	} else {
		SSD1306_DrawRectangle(current_x, small_segment_y, small_segment_width, small_segment_height, SSD1306_COLOR_WHITE);
	}

	if (g_settings.burn_in_protection) {
		battery_pos_x += battery_protection_step_x;
		if (battery_pos_x > screen_width - 4.75 * battery_segment_width - top_bar_padding) {
			battery_pos_x = top_bar_padding;
			battery_pos_y += battery_protection_step_y;
			if (battery_pos_y > top_bar_height - battery_segment_height - top_bar_padding) {
				battery_pos_y = top_bar_padding;
			}
		}
	}
}

void display_top_bar (uint8_t battery_percent) {
	display_clear_top_bar();
	display_battery(battery_percent);
	SSD1306_UpdateScreen();
}

void display_on(void) {
	SSD1306_ON();
}

void display_off(void) {
	SSD1306_OFF();
}
