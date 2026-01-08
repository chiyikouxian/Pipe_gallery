/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-05     ideapad15s       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "linesensor.h"
#include "hal_adc.h"

#define DBG_TAG "LineSensor"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * @brief ADC读取线程入口函数
 *
 * @param parameter 线程参数（未使用）
 */
/* 引脚配置参考linesensor.h */
static void adc_read_thread_entry(void *parameter)
{
    rt_uint32_t adc_ch0, adc_ch1, adc_ch3, adc_ch4, adc_ch5;

    while (1)
    {
        /* 使用 HAL 直接读取所有5个通道的ADC值 */
        adc_ch0 = ADC1_Read_Channel0();
        adc_ch1 = ADC1_Read_Channel1();
        adc_ch3 = ADC1_Read_Channel3();
        adc_ch4 = ADC1_Read_Channel4();
        adc_ch5 = ADC1_Read_Channel5();

        /* 串口输出ADC信息 */
        rt_kprintf("ADC CH0: %u  CH1: %u  CH3: %u  CH4: %u  CH5: %u\r\n",
                   (unsigned int)adc_ch0,
                   (unsigned int)adc_ch1,
                   (unsigned int)adc_ch3,
                   (unsigned int)adc_ch4,
                   (unsigned int)adc_ch5);
//         rt_kprintf("ADC CH5: %u\r\n",(unsigned int)adc_ch5);
        rt_kprintf("------------------------------------------\r\n");

        /* 延时500ms */
        rt_thread_mdelay(500);
    }
}

/**
 * @brief 初始化ADC1并启动读取线程
 *
 * @return rt_err_t RT_EOK表示成功，其他值表示失败
 */
rt_err_t line_sensor_init(void)
{
    rt_thread_t adc_thread;

    /* 初始化 HAL ADC1（底层 MSP 在 board.c 中实现） */
    ADC1_Init();

    /* 创建ADC读取线程 */
    adc_thread = rt_thread_create("adc_read",
                                   adc_read_thread_entry,
                                   RT_NULL,
                                   1024,
                                   25,
                                   10);

    if (adc_thread != RT_NULL)
    {
        rt_thread_startup(adc_thread);
        LOG_I("ADC read thread started (5 channels, HAL mode).");
    }
    else
    {
        LOG_E("Failed to create ADC read thread!");
        return -RT_ERROR;
    }

    return RT_EOK;
}
