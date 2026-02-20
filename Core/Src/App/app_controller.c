#include "App/app_controller.h"
#include "UI/ui_manager.h"
#include "Drivers/encoder.h"
#include "Display/display.h"
#include "Services/soft_timer.h"

// Keep battery update logic
soft_timer_t battery_update_timer;
const uint32_t battery_update_period = 15000;  // 15 seconds

void update_battery() {
	if (soft_timer_expired(&battery_update_timer)){
		// TODO: Here should be a voltage reading and a way to pass it to the UI
        display_top_bar(50);
		soft_timer_start(&battery_update_timer, battery_update_period); // 15 seconds
	}
}

void App_Init(void)
{
    display_init();
    Encoder_Init();
    UI_Init();
    soft_timer_start(&battery_update_timer, battery_update_period);
}

void App_Process(void)
{
    update_battery();
    UI_Render();

    // Now, check for encoder events and pass them to the UI
    App_Controls_Check();
}

void App_Controls_Check(void) {
    Encoder_ButtonTask(); // This needs to be called to update button state
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
    UI_HandleEvent(UI_EVENT_ROTATE_CW);
}

void App_Encoder_CCW(void)
{
    UI_HandleEvent(UI_EVENT_ROTATE_CCW);
}

void App_Encoder_Click(void)
{
    UI_HandleEvent(UI_EVENT_CLICK);
}

void App_Encoder_Long_Click(void)
{
    UI_HandleEvent(UI_EVENT_LONG_CLICK);
}

