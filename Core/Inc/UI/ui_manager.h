/*
 * ui_manager.h
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#ifndef INC_UI_UI_MANAGER_H_
#define INC_UI_UI_MANAGER_H_

#include "UI/menus/menu_types.h"

void UI_Init(void);
void UI_SetMenu(const menu_t *menu);
void UI_HandleEvent(ui_event_t event);
void UI_Render(void);

#endif /* INC_UI_UI_MANAGER_H_ */
