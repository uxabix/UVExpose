/*
 * app_controller.h
 *
 *  Created on: Jan 12, 2026
 *      Author: kiril
 */

#ifndef INC_APP_APP_CONTROLLER_H_
#define INC_APP_APP_CONTROLLER_H_

#include "main.h"

void App_Init(ADC_HandleTypeDef* hadc);
void App_Process(void);

void App_Encoder_CW(void);
void App_Encoder_CCW(void);
void App_Encoder_Click(void);
void App_Encoder_Long_Click(void);

#endif /* INC_APP_APP_CONTROLLER_H_ */
