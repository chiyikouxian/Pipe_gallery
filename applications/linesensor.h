/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-05     ideapad15s       the first version
 */
#ifndef APPLICATIONS_APPS_LINESENSOR_H_
#define APPLICATIONS_APPS_LINESENSOR_H_

#include <heads.h>

/* ADC device name */
#define ADC1_DEVICE_NAME    "adc1"

/* Five-channel flame sensor analog output mapping:
 * A1 -> PA0_C (ADC1_INP0)
 * A2 -> PA1_C (ADC1_INP1)
 * A3 -> PA6   (ADC1_INP3)
 * A4 -> PC4   (ADC1_INP4)
 * A5 -> PB1   (ADC1_INP5)
 */

extern BOOL Flame;          //火焰传感器 1有火 0无火

rt_uint32_t adc1_read_channel0(void);
rt_uint32_t adc1_read_channel1(void);
rt_uint32_t adc1_read_channel3(void);
rt_uint32_t adc1_read_channel4(void);
rt_uint32_t adc1_read_channel5(void);

/**
 * @brief Initialize ADC1 and start the five-channel flame sensor read thread
 *
 * @return rt_err_t RT_EOK表示成功，其他值表示失败
 */
rt_err_t line_sensor_init(void);


#endif /* APPLICATIONS_APPS_LINESENSOR_H_ */
