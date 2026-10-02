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
#include "huaweiCloudApp.h"  /* global ADC variables for cloud upload */
#include "NodeApp.h"         /* node[] structure for flame status */

BOOL Flame = 0;

#define DBG_TAG "LineSensor"
#define DBG_LVL DBG_ERROR
#include <rtdbg.h>

/**
 * @brief ADC��ȡ�߳���ں���
 *
 * @param parameter �̲߳�����δʹ�ã�
 */
/* Five-channel flame sensor analog output mapping:
 * A1 -> PA0_C
 * A2 -> PA1_C
 * A3 -> PA6
 * A4 -> PC4
 * A5 -> PB1
 */
static void adc_read_thread_entry(void *parameter)
{
    rt_uint32_t adc_ch0, adc_ch1, adc_ch3, adc_ch4, adc_ch5;
    rt_uint8_t fire_count;

    while (1)
    {
        /* ʹ�� HAL ֱ�Ӷ�ȡȫ��5��ͨ����ADCֵ */
        adc_ch0 = ADC1_Read_Channel0();
        adc_ch1 = ADC1_Read_Channel1();
        adc_ch3 = ADC1_Read_Channel3();
        adc_ch4 = ADC1_Read_Channel4();
        adc_ch5 = ADC1_Read_Channel5();

        /* Update global variables for cloud upload */
        g_adc_ch0 = adc_ch0;
        g_adc_ch1 = adc_ch1;
        g_adc_ch3 = adc_ch3;
        g_adc_ch4 = adc_ch4;
        g_adc_ch5 = adc_ch5;

        /* 火焰判断逻辑：4个及以上通道ADC值 > 60000 时判定为有火 */
        fire_count = 0;
        if (adc_ch0 > 60000) fire_count++;
        if (adc_ch1 > 60000) fire_count++;
        if (adc_ch3 > 60000) fire_count++;
        if (adc_ch4 > 60000) fire_count++;
        if (adc_ch5 > 60000) fire_count++;

        if (fire_count >= 4)
        {
            Flame = 1;  /* 有火 */
            node[0].Flame = 1;
        }
        else
        {
            Flame = 0;  /* 无火 */
            node[0].Flame = 0;
        }

        /* ��ʱ500ms */
        rt_thread_mdelay(500);
    }
}

/**
 * @brief ��ʼ��ADC1��������ȡ�߳�
 *
 * @return rt_err_t RT_EOK��ʾ�ɹ�������ֵ��ʾʧ��
 */
rt_err_t line_sensor_init(void)
{
    rt_thread_t adc_thread;

    /* ��ʼ�� HAL ADC1���ײ� MSP �� board.c ��ʵ�֣� */
    ADC1_Init();

    /* ����ADC��ȡ�߳� */
    adc_thread = rt_thread_create("adc_read",
                                   adc_read_thread_entry,
                                   RT_NULL,
                                   1024,
                                   25,
                                   10);

    if (adc_thread != RT_NULL)
    {
        rt_thread_startup(adc_thread);
        LOG_I("Five-channel flame sensor ADC read thread started (A1-A5, HAL mode).");
    }
    else
    {
        LOG_E("Failed to create ADC read thread!");
        return -RT_ERROR;
    }

    return RT_EOK;
}
