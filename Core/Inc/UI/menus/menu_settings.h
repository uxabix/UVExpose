/*
 * menu_settings.h
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#ifndef INC_UI_MENUS_MENU_SETTINGS_H_
#define INC_UI_MENUS_MENU_SETTINGS_H_

#include "UI/menus/menu_types.h"
#include <stdint.h>

extern const menu_t menu_settings;

// Getters for buzzer settings
uint8_t menu_settings_get_beep_count(void);
uint16_t menu_settings_get_beep_duration(void);
uint16_t menu_settings_get_beep_period(void);
uint8_t menu_settings_get_open_lid_protection(void);
uint16_t menu_settings_get_lid_open_threshold_mv(void);
uint16_t menu_settings_get_lid_close_threshold_mv(void);

void menu_settings_set_lid_open_threshold_mv(uint16_t mv);
void menu_settings_set_lid_close_threshold_mv(uint16_t mv);

#endif /* INC_UI_MENUS_MENU_SETTINGS_H_ */
