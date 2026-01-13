/*
 * display.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#include <stdbool.h>

#include "Display/display.h"
#include "Display/ssd1306.h"
#include "Display/fonts.h"

#define MENU_TEXT &Font_7x10, SSD1306_COLOR_WHITE
#define MENU_TEXT_HEIGHT 10
#define MENU_TEXT_WIDTH 7
#define MENU_PADDING 1

bool burnout_protection = true;

uint8_t screen_width = 128;
uint8_t screen_height = 64;

uint8_t top_bar_height = 8;
uint8_t top_bar_padding = 1;

uint8_t battery_pos_x = 0;
uint8_t battery_pos_y = 0;
const uint8_t battery_protection_step_x = 5;
const uint8_t battery_protection_step_y = 1;
const uint8_t battery_segment_height = 6;
const uint8_t battery_segment_width = 4;

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
	SSD1306_DrawFilledRectangle(0, top_bar_height + 1, screen_width, screen_height, SSD1306_COLOR_BLACK);
}

void display_timer() {

}

void display_menu(char (*options_list)[10], uint8_t options_count, uint8_t selected, uint8_t show_from) {
	display_clear_main_frame();
	char* current_option;
	uint16_t total_height = 2 + (MENU_TEXT_HEIGHT + 2) * options_count;
	uint8_t available_height = screen_height - top_bar_height;
	uint8_t current_y = top_bar_height + 1 + (available_height - total_height) / 2;
	if (total_height > available_height){

	}
	for (uint8_t i=0; i < options_count; i++) {
		SSD1306_GotoXY(MENU_PADDING, current_y);
		current_y += MENU_TEXT_HEIGHT + MENU_PADDING;
		current_option = options_list[i];
		SSD1306_Puts(current_option, MENU_TEXT);
	}
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
