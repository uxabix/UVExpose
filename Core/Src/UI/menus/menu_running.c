/*
 * menu_running.c
 *
 *  Created on: Feb 20, 2026
 *      Author: kiril
 */

#include "UI/menus/menu_running.h"
#include "UI/menus/menu_expose_options.h"
#include "UI/menus/menu_settings.h"
#include "UI/ui_manager.h"
#include "Display/display.h"
#include "Display/ssd1306.h"
#include "Display/fonts.h"
#include "UI/menus/menu_main.h"
#include "Services/exposure_service.h"
#include "Services/buzzer.h"
#include "Services/battery_service.h"

#include <string.h>
#include <stdio.h>

// UI local state only
static uint8_t is_infinite_mode = 0;  // 1 = "Active until off" mode, 0 = normal mode with timer
static uint8_t buzzer_notified = 0;   // Flag to track if buzzer was notified on timer finish
static uint8_t start_blocked_low_battery = 0; // 1 when start was blocked by battery policy

// Public function to set infinite mode (called from menu_expose_mode)
void menu_running_set_infinite_mode(uint8_t enabled)
{
    is_infinite_mode = enabled;
}

static void on_enter(void)
{
    start_blocked_low_battery = 0;

    BatteryService_Measure();
    if (!BatteryService_IsExposureAllowed()) {
        Exposure_Stop();
        Buzzer_Stop();
        start_blocked_low_battery = 1;
        return;
    }

    // Get settings from expose_options menu
    uint32_t exposure_time_ms = menu_expose_options_get_time_ms();
    uint8_t until_off = menu_expose_options_get_until_off();
    uint8_t beep = menu_expose_options_get_beep_mode();

    // Reset buzzer notification flag
    buzzer_notified = 0;

    // Configure buzzer service with the selected beep mode and pattern
    buzzer_mode_t buzzer_mode = (buzzer_mode_t)beep;
    Buzzer_SetMode(buzzer_mode);
    
    // Get buzzer pattern settings from menu_settings
    uint8_t beep_count = menu_settings_get_beep_count();
    uint16_t beep_duration = menu_settings_get_beep_duration();
    uint16_t beep_period = menu_settings_get_beep_period();
    Buzzer_SetBeepPattern(beep_count, beep_duration, beep_period);

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
    if (start_blocked_low_battery) {
        switch(event)
        {
            case UI_EVENT_CLICK:
                UI_SetMenu(&menu_expose_options);
                break;
            case UI_EVENT_LONG_CLICK:
                UI_SetMenu(&menu_main);
                break;
            default:
                break;
        }
        return;
    }

    switch(event)
    {
        case UI_EVENT_CLICK:
            // Toggle pause/resume in RUNNING state
            if (Exposure_IsRunning()) {
                Exposure_Pause();
                Buzzer_Stop();  // Stop buzzer when pausing
            } else if (Exposure_IsPaused()) {
                Exposure_Resume();
            } else if (Exposure_IsFinishedUntilOff()) {
                // In "Until off" mode after finish - turn off now
                Exposure_Stop();
                Buzzer_Stop();  // Stop buzzer
                UI_SetMenu(&menu_main);
            }
            break;

        case UI_EVENT_LONG_CLICK:
            // Cancel exposure and return to main menu
            Exposure_Stop();
            Buzzer_Stop();  // Stop buzzer
            UI_SetMenu(&menu_main);
            break;

        default:
            break;
    }
}

static void on_render(void)
{
    if (start_blocked_low_battery) {
        display_text_simple("Low batt, no UV");
        return;
    }

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
        // Notify buzzer on first transition to this state
        if (!buzzer_notified) {
            Buzzer_NotifyTimerFinished();
            buzzer_notified = 1;
        }
        display_text_simple("Press to end");
    } else if (Exposure_IsFinished()) {
        // Notify buzzer on first transition to this state
        if (!buzzer_notified) {
            Buzzer_NotifyTimerFinished();
            buzzer_notified = 1;
        }
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
