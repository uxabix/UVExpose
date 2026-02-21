/*
 * menu_time_select.h
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#ifndef INC_UI_MENUS_MENU_EXPOSE_OPTIONS_H_
#define INC_UI_MENUS_MENU_EXPOSE_OPTIONS_H_

#include "UI/menus/menu_types.h"
#include <stdint.h>

extern const menu_t menu_expose_options;

// Getters for exposure settings
uint32_t menu_expose_options_get_time_ms(void);
uint8_t menu_expose_options_get_beep_mode(void);
uint8_t menu_expose_options_get_until_off(void);

// Setters
void menu_expose_options_set_until_off(uint8_t enabled);
void menu_expose_options_set_beep_mode(uint8_t mode);
void menu_expose_options_set_time_ms(uint16_t time_ms);

// Debug: Reset to defaults
void menu_expose_options_reset_to_defaults(void);

#endif /* INC_UI_MENUS_MENU_EXPOSE_OPTIONS_H_ */
