/*
 * app_states.h
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#ifndef INC_APP_APP_STATES_H_
#define INC_APP_APP_STATES_H_

#include <stdint.h>
#include <stdbool.h>

/* ================================
 * Application states
 * ================================ */
typedef enum
{
    APP_STATE_INIT = 0,

    APP_STATE_IDLE,        // Waiting for user
    APP_STATE_ACTIVE_UI,   // Waiting for user input in UI
    APP_STATE_READY,       // Ready to start exposure
    APP_STATE_RUNNING,     // Exposure in progress
	APP_STATE_PAUSED,      // User pause
	APP_STATE_LID_OPEN,    //
    APP_STATE_DONE,        // Exposure finished
    APP_STATE_ERROR        // Any error state
} app_state_t;

typedef enum {
	UI_HOME,
	UI_MENU,
	UI_SETTINGS,
	UI_EDIT_TIME,
} ui_state_t;


/* ================================
 * Application events
 * ================================ */
typedef enum
{
    APP_EVENT_NONE = 0,

    /* User input */
    APP_EVENT_BTN_START_STOP,
    APP_EVENT_BTN_BACK,
    APP_EVENT_ENCODER_LEFT,
    APP_EVENT_ENCODER_RIGHT,
    APP_EVENT_ENCODER_PRESS,

    /* System events */
    APP_EVENT_LID_OPENED,
    APP_EVENT_LID_CLOSED,
    APP_EVENT_TIMER_EXPIRED,
    APP_EVENT_BATTERY_LOW,
    APP_EVENT_ERROR,

    /* Internal */
    APP_EVENT_TICK_1S

} app_event_t;

/* ================================
 * FSM public API
 * ================================ */

/* Initialize FSM */
void app_fsm_init(void);

/* Feed event to FSM */
void app_fsm_handle_event(app_event_t event);

/* Periodic processing (optional) */
void app_fsm_process(void);

/* Get current state */
app_state_t app_fsm_get_state(void);

/* Force state (for debug or recovery) */
void app_fsm_set_state(app_state_t state);


#endif /* INC_APP_APP_STATES_H_ */
