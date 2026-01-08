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

/* ADC设备名称  */
#define ADC1_DEVICE_NAME    "adc1"

//#define ADC_CHANNEL_0        0  /* PA0_C,  ADC12_INP0 */
//#define ADC_CHANNEL_1        1  /* PA1_C,  ADC12_INP1 */
//#define ADC_CHANNEL_3        3  /* PA6,    ADC12_INP3 */
//#define ADC_CHANNEL_4        4  /* PC4,    ADC12_INP4 */
//#define ADC_CHANNEL_5        5  /* PB1,    ADC12_INP5 */

extern BOOL Flame;          //火焰传感器 1有火 0无火

rt_uint32_t adc1_read_channel0(void);
rt_uint32_t adc1_read_channel1(void);
rt_uint32_t adc1_read_channel3(void);
rt_uint32_t adc1_read_channel4(void);
rt_uint32_t adc1_read_channel5(void);

/**
 * @brief 初始化ADC1并启动读取线程
 *
 * @return rt_err_t RT_EOK表示成功，其他值表示失败
 */
rt_err_t line_sensor_init(void);


#endif /* APPLICATIONS_APPS_LINESENSOR_H_ */
