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
const uint32_t battery_update_period = 15000;  // 15 seconds

soft_timer_t display_update_timer;
const uint32_t display_update_period = 900;

soft_timer_t running_timer; // New timer for the running state
const uint32_t running_timer_duration = 5 * 60 * 1000; // 5 minutes in milliseconds


void app_init(){
	display_init();
	display_clear();
	soft_timer_start(&battery_update_timer, battery_update_period);
	soft_timer_start(&display_update_timer, display_update_period);

	// DEBUG ONLY
	// Start the 5-minute timer and transition to RUNNING state
    soft_timer_start(&running_timer, running_timer_duration);
    app_state = APP_STATE_RUNNING; // Start in RUNNING state for demonstration
}
int choice = 0;
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
		if (soft_timer_expired(&display_update_timer)) {
			choice++;
			if (choice > 7) {
				choice = 0;
			}
			soft_timer_start(&display_update_timer, display_update_period);
		}

		char sample_menu[][16] = {
		    "Opt1",
		    "Longer",
		    "o3",
			"Opt4",
			"More options",
			"And another one",
			"One more",
			"Last one"
		};
		display_menu(sample_menu, 8, choice, choice);
		break;
	case APP_STATE_RUNNING:
		// Update top bar periodically
		if (soft_timer_expired(&battery_update_timer)){
			// TODO: Here should be a voltage reading
			display_top_bar(50);
			soft_timer_start(&battery_update_timer, battery_update_period); // 15 seconds
		}

		// Update and display timer every second
		if (soft_timer_expired(&display_update_timer)) {
			uint32_t current_tick = HAL_GetTick();
			uint32_t elapsed_ms = current_tick - running_timer.start;
			uint32_t remaining_ms = 0;

			if (elapsed_ms < running_timer.duration) {
				remaining_ms = running_timer.duration - elapsed_ms;
			} else {
				remaining_ms = 0; // Timer has expired
			}

			display_timer_remaining(remaining_ms);
			soft_timer_start(&display_update_timer, display_update_period); // Restart display update timer
		}

		// Check if the main running timer has expired
		if (soft_timer_expired(&running_timer)) {
			// Timer finished, transition to another state (e.g., menu)
			app_state = APP_STATE_ACTIVE_UI;
		}
		break; // End of APP_STATE_RUNNING
	case APP_STATE_DONE:
	case APP_STATE_ERROR:
	case APP_STATE_IDLE:
	case APP_STATE_LID_OPEN:
	case APP_STATE_PAUSED:
	case APP_STATE_READY:
		break;
	}
}

