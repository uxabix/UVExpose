/*
 * display.h
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#ifndef INC_DISPLAY_DISPLAY_H_
#define INC_DISPLAY_DISPLAY_H_

#include <stdint.h>

void display_init();
void display_top_bar (uint8_t battery_percent);
void display_clear();
void display_menu(char (*options_list)[10], uint8_t options_count, uint8_t selected, uint8_t show_from);

#endif /* INC_DISPLAY_DISPLAY_H_ */
