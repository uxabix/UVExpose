/*
 * adc_service.c
 *
 *  Created on: Feb 22, 2026
 *      Author: Gemini
 */

#include "Services/adc_service.h"
#include "adc.h"
#include "config.h"

static ADC_HandleTypeDef* s_hadc = NULL;

void AdcService_Init(ADC_HandleTypeDef* hadc) {
    s_hadc = hadc;
}

uint16_t AdcService_ReadChannel(uint32_t channel) {
    if (s_hadc == NULL) {
        return 0;
    }

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    // A long sampling time is good for noisy signals like battery voltage.
    // This can be made configurable if different channels need different times.
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    if (HAL_ADC_ConfigChannel(s_hadc, &sConfig) != HAL_OK) {
        // Error configuring channel. In a real scenario, you might want
        // to log this error or enter a fault state.
        return 0;
    }

    if (HAL_ADC_Start(s_hadc) != HAL_OK) {
    	return 0;
    }

    // Poll for conversion with a timeout of 10ms
    if (HAL_ADC_PollForConversion(s_hadc, 10) != HAL_OK) {
        HAL_ADC_Stop(s_hadc);
        return 0; // Return 0 on timeout/error
    }

    uint16_t raw_value = HAL_ADC_GetValue(s_hadc);

    // It's good practice to stop the ADC, though for single conversion mode
    // it might stop automatically. This ensures it's in a known state.
    HAL_ADC_Stop(s_hadc);

    return raw_value;
}

uint16_t AdcService_RawToMv(uint16_t raw_adc) {
    // Use a 64-bit intermediate value during multiplication to prevent overflow,
    // as (4095 * 3300) can exceed the range of a 32-bit integer.
    return ((uint64_t)raw_adc * ADC_VREF_MV) / ADC_RESOLUTION_MAX;
}
