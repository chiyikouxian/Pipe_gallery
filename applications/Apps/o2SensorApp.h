/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-04-22     ideapad15s   O2 sensor via ADC1 channel 8 (PC5)
 */
#ifndef APPLICATIONS_APPS_O2SENSORAPP_H_
#define APPLICATIONS_APPS_O2SENSORAPP_H_

#include <rtthread.h>

/* ADC config */
#define O2_ADC_DEVICE_NAME          "adc1"
#define O2_ADC_CHANNEL              8

/* Read interval (ms) — same as SHT30 */
#define O2_READ_INTERVAL_MS         2000

/* Calibration — adjust per sensor unit */
#define O2_ZERO_OFFSET_MV           0
#define O2_FULL_SCALE_MV            2410

/* Air-stabilize zone (×10 values) */
#define O2_AIR_STABILIZE_LOW        207
#define O2_AIR_STABILIZE_HIGH       211
#define O2_AIR_STABILIZE_VALUE      209

/* Global sensor data — O2 concentration in % */
extern float g_o2_concentration;

/* Thread entry */
void o2_thread_entry(void *parameter);

/* Init: create and start the O2 acquisition thread */
rt_err_t o2_sensor_init(void);

#endif /* APPLICATIONS_APPS_O2SENSORAPP_H_ */
