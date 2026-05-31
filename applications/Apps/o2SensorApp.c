/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-04-22     ideapad15s   O2 sensor via ADC1 channel 8 (PC5)
 */
#include "heads.h"
#include "o2SensorApp.h"

#define DBG_TAG "O2"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* Global sensor data */
float g_o2_concentration = 0.0f;

static rt_adc_device_t adc_dev = RT_NULL;

/**
 * @brief Read ADC raw value, convert to O2 concentration %.
 * @param o2_percent [out] O2 concentration in %
 * @return RT_EOK on success, negative on error
 */
static rt_err_t o2_read_once(float *o2_percent)
{
    rt_uint32_t raw;
    rt_uint32_t mV;
    rt_int32_t  o2_x10;

    raw = rt_adc_read(adc_dev, O2_ADC_CHANNEL);

    /* 12-bit ADC, 3.3V Vref: raw → mV */
    mV = raw * 3300 / 4096;

    /* Linear conversion: O2%(×10) = (mV - zero) * 209 / (full - zero) */
    o2_x10 = (rt_int32_t)(mV - O2_ZERO_OFFSET_MV) * 209
             / (rt_int32_t)(O2_FULL_SCALE_MV - O2_ZERO_OFFSET_MV);

    /* Air-stabilize: near 20.9% → force to 20.9% */
    if (o2_x10 >= O2_AIR_STABILIZE_LOW && o2_x10 <= O2_AIR_STABILIZE_HIGH)
    {
        o2_x10 = O2_AIR_STABILIZE_VALUE;
    }

    *o2_percent = (float)o2_x10 / 10.0f;
    return RT_EOK;
}

/**
 * @brief O2 sensor acquisition thread entry
 */
void o2_thread_entry(void *parameter)
{
    float o2;

    LOG_I("O2 thread started on %s ch%d", O2_ADC_DEVICE_NAME, O2_ADC_CHANNEL);

    while (1)
    {
        if (o2_read_once(&o2) == RT_EOK)
        {
            g_o2_concentration = o2;

            /* Print as scaled integer to avoid %f:
             * e.g. 209 → "20.9" */
            LOG_I("O2=%d.%d %%",
                  (int)o2, (int)((o2 - (int)o2) * 10));
        }
        else
        {
            LOG_W("Read failed, keeping last valid: O2=%d.%d %%",
                  (int)g_o2_concentration,
                  (int)((g_o2_concentration - (int)g_o2_concentration) * 10));
        }

        rt_thread_mdelay(O2_READ_INTERVAL_MS);
    }
}

/**
 * @brief Initialize O2 sensor: find ADC device, create and start thread
 */
rt_err_t o2_sensor_init(void)
{
    adc_dev = (rt_adc_device_t)rt_device_find(O2_ADC_DEVICE_NAME);
    if (adc_dev == RT_NULL)
    {
        LOG_E("ADC device %s not found", O2_ADC_DEVICE_NAME);
        return -RT_ERROR;
    }

    if (rt_adc_enable(adc_dev, O2_ADC_CHANNEL) != RT_EOK)
    {
        LOG_E("ADC enable ch%d failed", O2_ADC_CHANNEL);
        return -RT_ERROR;
    }

    rt_thread_t tid = rt_thread_create("o2_rd",
                                       o2_thread_entry,
                                       RT_NULL,
                                       1024,
                                       25,
                                       10);
    if (tid == RT_NULL)
    {
        LOG_E("Failed to create O2 thread");
        return -RT_ERROR;
    }

    rt_thread_startup(tid);
    LOG_I("O2 sensor initialized on %s ch%d", O2_ADC_DEVICE_NAME, O2_ADC_CHANNEL);
    return RT_EOK;
}
