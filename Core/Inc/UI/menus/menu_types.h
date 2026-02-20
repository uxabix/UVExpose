/*
 * menu_types.h
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#ifndef INC_UI_MENUS_MENU_TYPES_H_
#define INC_UI_MENUS_MENU_TYPES_H_


typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_ROTATE_CW,
    UI_EVENT_ROTATE_CCW,
    UI_EVENT_CLICK,
    UI_EVENT_LONG_CLICK
} ui_event_t;

typedef struct menu_s menu_t;

struct menu_s {
    void (*on_enter)(void);
    void (*on_event)(ui_event_t event);
    void (*on_render)(void);
};


#endif /* INC_UI_MENUS_MENU_TYPES_H_ */
