/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-04-22     ideapad15s   SHT30 temperature/humidity sensor via I2C1
 */
#include "heads.h"
#include "sht30App.h"

#define DBG_TAG "SHT30"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* SHT30 CRC8: polynomial 0x31, init 0xFF */
static uint8_t sht30_crc8(const uint8_t *data, int len)
{
    uint8_t crc = 0xFF;
    int i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* Global sensor data */
float g_temperature_c = 0.0f;
float g_humidity_rh    = 0.0f;

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/**
 * @brief Send measurement command and read 6 bytes from SHT30.
 * @param temp_c  [out] parsed temperature in Celsius
 * @param humi_rh [out] parsed relative humidity in %
 * @return RT_EOK on success, negative on error
 */
static rt_err_t sht30_read_once(float *temp_c, float *humi_rh)
{
    uint8_t cmd[2] = SHT30_CMD_MEAS_HIGH_REP;
    uint8_t buf[6];
    struct rt_i2c_msg msgs[1];
    rt_uint16_t raw_t, raw_h;
    rt_err_t ret;

    /* Send measurement command */
    msgs[0].addr  = SHT30_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = cmd;
    msgs[0].len   = 2;

    ret = rt_i2c_transfer(i2c_bus, msgs, 1);
    if (ret != 1)
    {
        LOG_E("I2C write cmd failed, ret=%d", ret);
        return -RT_ERROR;
    }

    /* Wait for measurement to complete (>15ms for high repeatability) */
    rt_thread_mdelay(20);

    /* Read 6 bytes: temp MSB, temp LSB, temp CRC, humi MSB, humi LSB, humi CRC */
    msgs[0].addr  = SHT30_I2C_ADDR;
    msgs[0].flags = RT_I2C_RD;
    msgs[0].buf   = buf;
    msgs[0].len   = 6;

    ret = rt_i2c_transfer(i2c_bus, msgs, 1);
    if (ret != 1)
    {
        LOG_E("I2C read data failed, ret=%d", ret);
        return -RT_ERROR;
    }

    /* CRC8 verification on temperature bytes (buf[0..1] vs buf[2]) */
    if (sht30_crc8(buf, 2) != buf[2])
    {
        LOG_E("CRC8 fail on temperature bytes: %02X %02X %02X",
              buf[0], buf[1], buf[2]);
        return -RT_ERROR;
    }

    /* CRC8 verification on humidity bytes (buf[3..4] vs buf[5]) */
    if (sht30_crc8(&buf[3], 2) != buf[5])
    {
        LOG_E("CRC8 fail on humidity bytes: %02X %02X %02X",
              buf[3], buf[4], buf[5]);
        return -RT_ERROR;
    }

    raw_t = ((rt_uint16_t)buf[0] << 8) | buf[1];
    raw_h = ((rt_uint16_t)buf[3] << 8) | buf[4];

    *temp_c  = -45.0f + 175.0f * (float)raw_t / 65535.0f;
    *humi_rh = 100.0f * (float)raw_h / 65535.0f;

    return RT_EOK;
}

/**
 * @brief SHT30 acquisition thread entry
 */
void sht30_thread_entry(void *parameter)
{
    float temp, humi;

    LOG_I("SHT30 thread started on %s", SHT30_I2C_BUS_NAME);

    while (1)
    {
        if (sht30_read_once(&temp, &humi) == RT_EOK)
        {
            g_temperature_c = temp;
            g_humidity_rh    = humi;

            /* Print as scaled integers to avoid %f */
            LOG_I("T=%d.%02d C, RH=%d.%02d %%",
                  (int)temp, (int)((temp - (int)temp) * 100),
                  (int)humi, (int)((humi - (int)humi) * 100));
        }
        else
        {
            LOG_W("Read failed, keeping last valid: T=%d.%02d C, RH=%d.%02d %%",
                  (int)g_temperature_c,
                  (int)((g_temperature_c - (int)g_temperature_c) * 100),
                  (int)g_humidity_rh,
                  (int)((g_humidity_rh - (int)g_humidity_rh) * 100));
        }

        rt_thread_mdelay(SHT30_READ_INTERVAL_MS);
    }
}

/**
 * @brief Initialize SHT30: find I2C bus, create and start thread
 */
rt_err_t sht30_init(void)
{
    i2c_bus = rt_i2c_bus_device_find(SHT30_I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        LOG_E("I2C bus %s not found", SHT30_I2C_BUS_NAME);
        return -RT_ERROR;
    }

    rt_thread_t tid = rt_thread_create("sht30_rd",
                                       sht30_thread_entry,
                                       RT_NULL,
                                       1024,
                                       25,
                                       10);
    if (tid == RT_NULL)
    {
        LOG_E("Failed to create SHT30 thread");
        return -RT_ERROR;
    }

    rt_thread_startup(tid);
    LOG_I("SHT30 initialized on %s", SHT30_I2C_BUS_NAME);
    return RT_EOK;
}
