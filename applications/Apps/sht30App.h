/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-04-22     ideapad15s   SHT30 temperature/humidity sensor via I2C1
 */
#ifndef APPLICATIONS_APPS_SHT30APP_H_
#define APPLICATIONS_APPS_SHT30APP_H_

#include <rtthread.h>

/* I2C config */
#define SHT30_I2C_BUS_NAME          "i2c1"
#define SHT30_I2C_ADDR              0x44

/* Measurement command: single-shot, high repeatability */
#define SHT30_CMD_MEAS_HIGH_REP     {0x2C, 0x06}

/* Read interval (ms) */
#define SHT30_READ_INTERVAL_MS      2000

/* Global sensor data */
extern float g_temperature_c;   /* Temperature in Celsius */
extern float g_humidity_rh;     /* Relative humidity in % */

/* Thread entry */
void sht30_thread_entry(void *parameter);

/* Init: create and start the SHT30 acquisition thread */
rt_err_t sht30_init(void);

#endif /* APPLICATIONS_APPS_SHT30APP_H_ */
