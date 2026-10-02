/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-05     ideapad15s       the first version
 */
#ifndef APPLICATIONS_HAL_ADC_H_
#define APPLICATIONS_HAL_ADC_H_

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_adc.h"
#include "stm32h7xx_hal_adc_ex.h"

extern ADC_HandleTypeDef hadc1;

void ADC1_Init(void);

uint32_t ADC1_Read_Channel0(void);
uint32_t ADC1_Read_Channel1(void);
uint32_t ADC1_Read_Channel3(void);
uint32_t ADC1_Read_Channel4(void);
uint32_t ADC1_Read_Channel5(void);

#endif /* APPLICATIONS_HAL_ADC_H_ */
