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

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define MENU_FONT &Font_7x10
#define MENU_COLOR_DEFAULT SSD1306_COLOR_WHITE
#define MENU_TEXT MENU_FONT, MENU_COLOR_DEFAULT
#define MENU_TEXT_HEIGHT 10
#define MENU_TEXT_WIDTH 7
#define MENU_PADDING 1

bool burnout_protection = true;

uint8_t screen_width = 128;
uint8_t screen_height = 64;

uint8_t top_bar_height = 9;
uint8_t top_bar_padding = 1;

uint8_t battery_pos_x = 0;
uint8_t battery_pos_y = 0;
const uint8_t battery_protection_step_x = 5;
const uint8_t battery_protection_step_y = 1;
const uint8_t battery_segment_height = 6;
const uint8_t battery_segment_width = 4;

// Display Menu configuration
const uint8_t HIGHLIGHT_PADDING_X = 2;
const uint8_t HIGHLIGHT_PADDING_Y = 1;
const uint8_t SCROLLBAR_WIDTH = 4;
const uint8_t SCROLLBAR_MARGIN = 1; // From right side

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


// Display Timer configuration
#define TIMER_FONT &Font_16x26 // Or define a new font
#define TIMER_TEXT_HEIGHT 26 // Or define a new height
#define TIMER_TEXT_WIDTH 16 // Or define a new width
#define TIMER_FLOAT_STEP_X 1 // Pixels to move per update
#define TIMER_FLOAT_STEP_Y 1 // Pixels to move per update
#define TIMER_FLOAT_MARGIN 5 // Margin from display area edges

// Static variables for floating text position
static int16_t timer_current_x = -1; // -1 indicates uninitialized
static int16_t timer_current_y = -1;
static int8_t timer_dx = TIMER_FLOAT_STEP_X;
static int8_t timer_dy = TIMER_FLOAT_STEP_Y;

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
	SSD1306_DrawFilledRectangle(0, top_bar_height - 1, screen_width, screen_height - top_bar_height, SSD1306_COLOR_BLACK);
}

void display_timer_remaining(uint32_t remaining_ms) {
    char time_str[6]; // MM:SS\0 (e.g., "00:00")
    uint32_t total_seconds = remaining_ms / 1000;
    uint8_t minutes = total_seconds / 60;
    uint8_t seconds = total_seconds % 60;

    sprintf(time_str, "%02d:%02d", minutes, seconds);

    uint16_t text_width = strlen(time_str) * TIMER_TEXT_WIDTH;
    uint8_t text_height = TIMER_TEXT_HEIGHT;

    // Define the floating area boundaries
    uint8_t float_area_min_x = TIMER_FLOAT_MARGIN;
    uint8_t float_area_max_x = screen_width - TIMER_FLOAT_MARGIN - text_width;
    uint8_t float_area_min_y = top_bar_height + TIMER_FLOAT_MARGIN;
    uint8_t float_area_max_y = screen_height - TIMER_FLOAT_MARGIN - text_height;

    // Initialize position if not already set
    if (timer_current_x == -1 || timer_current_y == -1) {
        timer_current_x = (float_area_min_x + float_area_max_x) / 2;
        timer_current_y = (float_area_min_y + float_area_max_y) / 2;
    }

    // Update position
    timer_current_x += timer_dx;
    timer_current_y += timer_dy;

    // Check boundaries and reverse direction
    if (timer_current_x < float_area_min_x) {
        timer_current_x = float_area_min_x;
        timer_dx = TIMER_FLOAT_STEP_X;
    } else if (timer_current_x > float_area_max_x) {
        timer_current_x = float_area_max_x;
        timer_dx = -TIMER_FLOAT_STEP_X;
    }

    if (timer_current_y < float_area_min_y) {
        timer_current_y = float_area_min_y;
        timer_dy = TIMER_FLOAT_STEP_Y;
    } else if (timer_current_y > float_area_max_y) {
        timer_current_y = float_area_max_y;
        timer_dy = -TIMER_FLOAT_STEP_Y;
    }

    // Clear the main frame before drawing to avoid trails
    display_clear_main_frame();

    // Draw the time string
    SSD1306_GotoXY(timer_current_x, timer_current_y);
    SSD1306_Puts(time_str, TIMER_FONT, SSD1306_COLOR_WHITE);

    // SSD1306_UpdateScreen() will be called by the caller (e.g., app_process)
    // or if this is the only thing on screen, it can be called here.
    // Given the context, display_menu calls it, display_top_bar calls it.
    // So, it's better to call it here if this function is the primary display update.
    // If it's part of a larger update cycle, the caller should call it.
    // The prompt says "вызываться она будет раз в секунду из другого файла - т.е. она не ответсвенна за отслеживание обновлений на таймере"
    // This implies it's a standalone drawing function. So, it should call UpdateScreen.
    SSD1306_UpdateScreen();
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
static void display_menu_draw_items(char display_strings[][16 + 1], uint8_t options_count, uint8_t selected, MenuLayout_t* layout, uint16_t current_max_item_text_width) {
    // Calculate width and X-position for the uniform highlight rectangle
    layout->uniform_rect_width = current_max_item_text_width + (2 * HIGHLIGHT_PADDING_X);
    layout->uniform_rect_x = (layout->effective_screen_width - layout->uniform_rect_width) / 2;
    layout->uniform_rect_x = MAX(0, layout->uniform_rect_x); // Ensure X is not negative

    uint8_t current_y = layout->start_y;

    // Iterate through visible menu items
    for (uint8_t i = 0; i < layout->actual_visible_items; i++) {
        uint8_t option_index = layout->clamped_scroll_offset + i;
        char* current_option_to_display = display_strings[i];
        uint16_t current_item_text_width = strlen(current_option_to_display) * MENU_TEXT_WIDTH;

        // Highlight selected item
        if (option_index == selected) {
            uint8_t rect_x = layout->uniform_rect_x;
            uint8_t rect_y = current_y - HIGHLIGHT_PADDING_Y;
            uint8_t rect_width = layout->uniform_rect_width;
            uint8_t rect_height = MENU_TEXT_HEIGHT + (2 * HIGHLIGHT_PADDING_Y);

            // Draw filled rectangle (white color)
            SSD1306_DrawFilledRectangle(rect_x, rect_y, rect_width, rect_height, SSD1306_COLOR_WHITE);
            // Draw text in black on white background, centered within the highlight
            SSD1306_GotoXY(layout->uniform_rect_x + HIGHLIGHT_PADDING_X + (current_max_item_text_width - current_item_text_width) / 2, current_y);
            SSD1306_Puts(current_option_to_display, MENU_FONT, SSD1306_COLOR_BLACK);
        } else {
            // Draw unselected item in white, centered relative to the uniform highlight
            SSD1306_GotoXY(layout->uniform_rect_x + HIGHLIGHT_PADDING_X + (current_max_item_text_width - current_item_text_width) / 2, current_y);
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
void display_menu(char (*options_list)[16], uint8_t options_count, uint8_t selected, uint8_t scroll_offset) {
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

    display_menu_draw_items(display_strings, options_count, selected, &layout, current_max_item_text_width);
    display_menu_draw_scrollbar(options_count, &layout);

    SSD1306_UpdateScreen();
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

	if (burnout_protection) {
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
