/*
 * display.h
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#ifndef INC_DISPLAY_DISPLAY_H_
#define INC_DISPLAY_DISPLAY_H_

#include <stdint.h>

// Font size enum for display_text_simple_sized
typedef enum {
    FONT_SIZE_SMALL,   // 7x10
    FONT_SIZE_MEDIUM,  // 11x18
    FONT_SIZE_LARGE    // 16x26
} FontSize_t;

void display_init();
void display_top_bar (uint8_t battery_percent);
void display_clear();
void display_text_simple(const char* text);
void display_text_simple_sized(const char* text, FontSize_t font_size);
void display_timer_remaining(uint32_t remaining_ms);
void display_menu(char (*options_list)[16], uint8_t options_count, uint8_t selected, uint8_t show_from);
void display_menu_column(char (*options_list)[16], uint8_t options_count, uint8_t selected, uint8_t scroll_offset, uint8_t selected_column_index);

void display_on(void);
void display_off(void);

#endif /* INC_DISPLAY_DISPLAY_H_ */
