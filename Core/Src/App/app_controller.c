/*
 * app_controller.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#include <stdint.h>

#include "App/app_controller.h"
#include "Display/display.h"

void app_init(){
	display_init();
	display_clear();
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
	for (uint8_t num=0; num<=100; num+=10) {
		display_top_bar(num);
		HAL_Delay(1000);
	}
//	for (int num=1;num<=1000;num++)
//	{
//		itoa(num,string, 10);
//		SSD1306_GotoXY (0, 30);
//		SSD1306_Puts ("             ", &Font_16x26, 1);
//		SSD1306_UpdateScreen();
//		if(num<10) {
//			SSD1306_GotoXY (53, 30);  // 1 DIGIT
//		}
//		else if (num<100) {
//			SSD1306_GotoXY (45, 30);  // 2 DIGITS
//		}
//		else  {
//			SSD1306_GotoXY (37, 30);  // 3 DIGITS
//		}
//		SSD1306_Puts (string, &Font_16x26, 1);
//		SSD1306_UpdateScreen();
//		HAL_Delay (500);
//	}
}

