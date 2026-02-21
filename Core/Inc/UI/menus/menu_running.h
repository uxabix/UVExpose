/*
 * menu_running.h
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#ifndef INC_UI_MENUS_MENU_RUNNING_H_
#define INC_UI_MENUS_MENU_RUNNING_H_

#include "UI/menus/menu_types.h"
#include <stdint.h>

extern const menu_t menu_running;

// Set infinite mode (Active until off - no timer)
void menu_running_set_infinite_mode(uint8_t enabled);

#endif /* INC_UI_MENUS_MENU_RUNNING_H_ */
