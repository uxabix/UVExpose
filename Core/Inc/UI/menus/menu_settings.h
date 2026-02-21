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

#endif /* INC_UI_MENUS_MENU_SETTINGS_H_ */
