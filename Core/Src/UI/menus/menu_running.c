/*
 * menu_running.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_running.h"
#include "UI/menus/menu_expose_options.h"
#include "UI/ui_manager.h"
#include "Display/display.h"
#include "Display/ssd1306.h"
#include "Display/fonts.h"
#include "UI/menus/menu_main.h"
#include "App/exposure_service.h"

#include <string.h>
#include <stdio.h>

// UI local state only
static uint8_t is_infinite_mode = 0;  // 1 = "Active until off" mode, 0 = normal mode with timer

// Public function to set infinite mode (called from menu_expose_mode)
void menu_running_set_infinite_mode(uint8_t enabled)
{
    is_infinite_mode = enabled;
}

static void on_enter(void)
{
    // Get settings from expose_options menu
    uint32_t exposure_time_ms = menu_expose_options_get_time_ms();
    uint8_t until_off = menu_expose_options_get_until_off();
    uint8_t beep = menu_expose_options_get_beep_mode();

    // In infinite mode, use 0 (unlimited) time
    if (is_infinite_mode) {
        Exposure_Start(0, 0, beep);  // 0 time = unlimited, no until_off needed
    } else {
        // Normal mode: start with timer
        Exposure_Start(exposure_time_ms, until_off, beep);
    }
}

static void on_event(ui_event_t event)
{
    switch(event)
    {
        case UI_EVENT_CLICK:
            // Toggle pause/resume in RUNNING state
            if (Exposure_IsRunning()) {
                Exposure_Pause();
            } else if (Exposure_IsPaused()) {
                Exposure_Resume();
            } else if (Exposure_IsFinishedUntilOff()) {
                // In "Until off" mode after finish - turn off now
                Exposure_Stop();
                UI_SetMenu(&menu_main);
            }
            break;

        case UI_EVENT_LONG_CLICK:
            // Cancel exposure and return to main menu
            Exposure_Stop();
            UI_SetMenu(&menu_main);
            break;

        default:
            break;
    }
}

static void on_render(void)
{
    // In infinite mode, show "Active" with floating animation
    if (is_infinite_mode && Exposure_IsRunning()) {
        display_timer_remaining(0xFFFFFFFF); // This will show the floating text "Active"
        return;
    }

    // Display based on exposure service state
    if (Exposure_IsPaused()) {
        display_text_simple("Paused");
    } else if (Exposure_IsRunning()) {
        display_timer_remaining(Exposure_GetRemainingTime());
    } else if (Exposure_IsFinishedUntilOff()) {
        display_text_simple("Press to end");
    } else if (Exposure_IsFinished()) {
        display_text_simple("Finished");
    } else {
        display_text_simple("Stopped");
    }
}

const menu_t menu_running = {
    .on_enter = on_enter,
    .on_event = on_event,
    .on_render = on_render
};
