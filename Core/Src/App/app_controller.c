#include "App/app_controller.h"
#include "App/app_states.h"
#include "Services/exposure_service.h"
#include "Services/settings_service.h"
#include "Services/presets_service.h"
#include "UI/ui_manager.h"
#include "Drivers/encoder.h"
#include "Display/display.h"
#include "Services/soft_timer.h"
#include "Services/buzzer.h"
#include "Safety/safety_manager.h"
#include "Services/power_manager.h"
#include "Services/battery_service.h"
#include "Services/adc_service.h"

// ========== FSM State Machine ==========
static app_state_t current_state = APP_STATE_INIT;
static app_event_t pending_event = APP_EVENT_NONE;

// Keep battery update logic
static soft_timer_t battery_update_timer;
static const uint32_t battery_update_period = 15000;  // 15 seconds
static uint8_t critical_sleep_pending = 0u;
static uint32_t critical_sleep_started_at_ms = 0u;

// ========== FSM Public API Implementation ==========

void app_fsm_init(void)
{
    current_state = APP_STATE_IDLE;
    pending_event = APP_EVENT_NONE;
}

void app_fsm_set_state(app_state_t state)
{
    current_state = state;
}

app_state_t app_fsm_get_state(void)
{
    return current_state;
}

void app_fsm_handle_event(app_event_t event)
{
    pending_event = event;
}

void app_fsm_process(void)
{
    if(pending_event == APP_EVENT_NONE)
        return;

    app_event_t event = pending_event;
    pending_event = APP_EVENT_NONE;

    switch(current_state)
    {
        case APP_STATE_INIT:
            // Initialization complete, transition to IDLE
            if(event == APP_EVENT_TICK_1S) {
                current_state = APP_STATE_IDLE;
            }
            break;

        case APP_STATE_IDLE:
            // In IDLE, waiting for user to navigate menu
            if(event == APP_EVENT_BTN_START_STOP) {
                current_state = APP_STATE_ACTIVE_UI;
            }
            break;

        case APP_STATE_ACTIVE_UI:
            // User is interacting with UI
            if(event == APP_EVENT_BTN_BACK) {
                current_state = APP_STATE_IDLE;
            }
            // Navigation events handled by UI layer
            break;

        case APP_STATE_READY:
            // Ready to start exposure
            if(event == APP_EVENT_BTN_START_STOP) {
                current_state = APP_STATE_RUNNING;
            } else if(event == APP_EVENT_BTN_BACK) {
                current_state = APP_STATE_ACTIVE_UI;
            }
            break;

        case APP_STATE_RUNNING:
            // Exposure in progress
            if(event == APP_EVENT_BTN_START_STOP) {
                current_state = APP_STATE_PAUSED;
            } else if(event == APP_EVENT_LID_OPENED) {
                current_state = APP_STATE_LID_OPEN;
            } else if(event == APP_EVENT_TIMER_EXPIRED) {
                current_state = APP_STATE_DONE;
            } else if(event == APP_EVENT_ERROR) {
                current_state = APP_STATE_ERROR;
            }
            break;

        case APP_STATE_PAUSED:
            // User paused exposure
            if(event == APP_EVENT_BTN_START_STOP) {
                current_state = APP_STATE_RUNNING;
            } else if(event == APP_EVENT_BTN_BACK) {
                current_state = APP_STATE_IDLE;
            } else if(event == APP_EVENT_LID_OPENED) {
                current_state = APP_STATE_LID_OPEN;
            }
            break;

        case APP_STATE_LID_OPEN:
            // Lid is open during exposure
            if(event == APP_EVENT_LID_CLOSED) {
                current_state = APP_STATE_RUNNING;
            }
            break;

        case APP_STATE_DONE:
            // Exposure finished
            if(event == APP_EVENT_BTN_START_STOP || event == APP_EVENT_BTN_BACK) {
                current_state = APP_STATE_IDLE;
            }
            break;

        case APP_STATE_ERROR:
            // Error occurred
            if(event == APP_EVENT_BTN_BACK) {
                current_state = APP_STATE_IDLE;
            }
            break;

        case APP_STATE_ENCODER_TEST:
            // Encoder testing mode
            if(event == APP_EVENT_BTN_BACK) {
                current_state = APP_STATE_IDLE;
            }
            break;

        default:
            break;
    }
}

// ========== Battery Update ==========


void update_battery() {
	if (soft_timer_expired(&battery_update_timer)){
		BatteryService_Measure();
        display_top_bar(BatteryService_GetPercentage());
		soft_timer_start(&battery_update_timer, battery_update_period);
	}
}

static uint8_t enforce_critical_battery_sleep(void)
{
#if (IGNORE_BATTERY_CRITICAL == 1)
    critical_sleep_pending = 0u;
    return 0u;
#else
    BatteryService_Measure();
    if (BatteryService_GetStatus() != BATTERY_STATUS_CRITICAL) {
        critical_sleep_pending = 0u;
        return 0u;
    }

    Exposure_Stop();
    Buzzer_Stop();

    if (!critical_sleep_pending) {
        critical_sleep_pending = 1u;
        critical_sleep_started_at_ms = HAL_GetTick();
    }

    display_text_simple("Battery critical");

    if ((HAL_GetTick() - critical_sleep_started_at_ms) >= BATTERY_CRITICAL_SLEEP_DELAY_MS) {
        critical_sleep_pending = 0u;
        power_manager_sleep();
        BatteryService_Measure();
        display_top_bar(BatteryService_GetPercentage());
    }

    return 1u;
#endif
}

void App_Init(ADC_HandleTypeDef* hadc)
{
    power_manager_debug_startup_blink();

	AdcService_Init(hadc);
	Settings_Init();
    Presets_Init();
	BatteryService_Init();
    display_init();
    Encoder_Init();
    UI_Init();
    Exposure_Init();  // Initialize exposure service
    Buzzer_Init();    // Initialize buzzer service
    Safety_Init();    // Initialize safety manager (lid sensor)
    app_fsm_init();
    power_manager_notify_activity(); // Initialize activity tick
    soft_timer_start(&battery_update_timer, 100); // Trigger first update quickly
}

void App_Process(void)
{
    update_battery();
    if (enforce_critical_battery_sleep()) {
        Encoder_ButtonTick(); // Keep button timing coherent while warning is displayed
        return;
    }
    UI_Render();
    Safety_Process();  // Check lid sensor and react if needed
    Exposure_Process();  // Process exposure service
    Buzzer_Process();    // Process buzzer service

    // Process FSM
    app_fsm_process();

    // Check for encoder events and pass them to the UI
    App_Controls_Check();

    power_manager_update(); // Check inactivity sleep
    Encoder_ButtonTick(); // Check for long press while button is held
}

void App_Controls_Check(void) {
    encoder_direction_t dir = Encoder_GetDirection();
    if(dir == ENCODER_CW) {
        App_Encoder_CW();
    } else if(dir == ENCODER_CCW) {
        App_Encoder_CCW();
    }

    if(Encoder_ButtonLongPressed()) {
        App_Encoder_Long_Click();
    } else if(Encoder_ButtonPressed()) {
        App_Encoder_Click();
    }
}

void App_Encoder_CW(void)
{
    power_manager_notify_activity();
    UI_HandleEvent(UI_EVENT_ROTATE_CW);
    // Encoder rotation in IDLE state - no FSM event needed
}

void App_Encoder_CCW(void)
{
    power_manager_notify_activity();
    UI_HandleEvent(UI_EVENT_ROTATE_CCW);
    // Encoder rotation in IDLE state - no FSM event needed
}

void App_Encoder_Click(void)
{
    power_manager_notify_activity();
    UI_HandleEvent(UI_EVENT_CLICK);
    // Normal click maps to BTN_START_STOP event
    app_fsm_handle_event(APP_EVENT_BTN_START_STOP);
}

void App_Encoder_Long_Click(void)
{
    power_manager_notify_activity();
    UI_HandleEvent(UI_EVENT_LONG_CLICK);
    // Long click maps to BTN_BACK event
    app_fsm_handle_event(APP_EVENT_BTN_BACK);
}

