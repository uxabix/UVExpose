/*
 * app_controller.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#include <stdint.h>

#include "App/app_controller.h"
#include "Display/display.h"
#include "App/app_states.h"
#include "Services/soft_timer.h"

app_state_t app_state = APP_STATE_INIT;
soft_timer_t battery_update_timer;
const uint32_t battery_update_period = 15000;

void app_init(){
	display_init();
	display_clear();
	soft_timer_start(&battery_update_timer, battery_update_period); // 15 seconds
//	char string[5];
//
//	SSD1306_GotoXY (0,0);
//	SSD1306_Puts ("SSD1306", &Font_11x18, 1);
//	SSD1306_GotoXY (0, 30);
//	SSD1306_Puts ("OLED DEMO", &Font_11x18, 1);
//	SSD1306_UpdateScreen();
//
//	HAL_Delay(3000);
//	clear_display();
//	SSD1306_GotoXY (30,0);
//	SSD1306_Puts ("COUNTER", &Font_11x18, 1);
}

void app_process() {
	switch (app_state) {
	case APP_STATE_INIT:
		// TODO: Here should be a voltage reading
		display_top_bar(50);
		app_state = APP_STATE_ACTIVE_UI;
		break;
	case APP_STATE_ACTIVE_UI:
		if (soft_timer_expired(&battery_update_timer)){
			// TODO: Here should be a voltage reading
			display_top_bar(50);
			soft_timer_start(&battery_update_timer, battery_update_period); // 15 seconds
		}
		break;
	case APP_STATE_DONE:
	case APP_STATE_ERROR:
	case APP_STATE_IDLE:
	case APP_STATE_LID_OPEN:
	case APP_STATE_PAUSED:
	case APP_STATE_READY:
	case APP_STATE_RUNNING:
	}
}

