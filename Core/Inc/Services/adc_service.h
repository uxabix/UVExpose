/*
 * adc_service.h
 *
 *  Created on: Feb 22, 2026
 *      Author: Gemini
 *
 *  A centralized service to manage access to the ADC peripheral,
 *  preventing contention between different application modules.
 */

#ifndef INC_SERVICES_ADC_SERVICE_H_
#define INC_SERVICES_ADC_SERVICE_H_

#include "main.h"
#include <stdint.h>


/**
 * @brief Initializes the ADC service.
 *
 * @param hadc Pointer to the ADC handle for the application.
 */
void AdcService_Init(ADC_HandleTypeDef* hadc);

/**
 * @brief Reads a raw value from a specific ADC channel.
 * This function handles channel selection, conversion, and returns the raw value.
 * It is a blocking call.
 *
 * @param channel The ADC channel to read (e.g., ADC_CHANNEL_0).
 * @return The 12-bit raw ADC result, or 0 on error/timeout.
 */
uint16_t AdcService_ReadChannel(uint32_t channel);

/**
 * @brief Converts a raw ADC value to millivolts.
 * This is a utility function to centralize the conversion logic based
 * on the system's VREF and ADC resolution defined in config.h.
 *
 * @param raw_adc The raw value from an ADC conversion.
 * @return The corresponding voltage in millivolts (mV).
 */
uint16_t AdcService_RawToMv(uint16_t raw_adc);

#endif /* INC_SERVICES_ADC_SERVICE_H_ */
