/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-04-13     ideapad15s   ATK-LORA-01 module via UART5
 */
#include "heads.h"
#include "loraApp.h"
#include "huaweiCloudApp.h"
#include "MethaneSensorApp.h"
#include "freeModbusApp.h"
#include "uartApp.h"

#define DBG_TAG "LoRa"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LORA_TEXT_BUF_SIZE 256

static int abs_int(int value)
{
    return value < 0 ? -value : value;
}

/**
 * @brief Build LoRa text payload in requested format.
 *
 * Format:
 * Flame: xx,xx,xx,xx,xx
 * gas: ppm: xx , level: xx%
 * voltage_a: xxV,voltage_b: xxV,voltage_c: xxV
 * current_a: xxA,current_b: xxA,current_c: xxA
 * flow: xxm³/h
 */
static void lora_build_text(char *buf, rt_size_t size)
{
    int voltage_a_int = (int)Voltage[0];
    int voltage_b_int = (int)Voltage[1];
    int voltage_c_int = (int)Voltage[2];
    int current_a_int = (int)Current[0];
    int current_b_int = (int)Current[1];
    int current_c_int = (int)Current[2];
    int flow_int = (int)Flow;

    rt_snprintf(buf, size,
                "Flame: %u,%u,%u,%u,%u\r\n"
                "gas: ppm: %u , level: %u%%\r\n"
                "voltage_a: %dV,voltage_b: %dV,voltage_c: %dV\r\n"
                "current_a: %dA,current_b: %dA,current_c: %dA\r\n"
                "flow: %d m^3/h\r\n",
                (unsigned int)g_adc_ch0,
                (unsigned int)g_adc_ch1,
                (unsigned int)g_adc_ch3,
                (unsigned int)g_adc_ch4,
                (unsigned int)g_adc_ch5,
                (unsigned int)g_methane_ppm,
                (unsigned int)g_methane_lel,
                voltage_a_int,
                voltage_b_int,
                voltage_c_int,
                current_a_int,
                current_b_int,
                current_c_int,
                flow_int);
}

/**
 * @brief LoRa send thread entry
 *
 * Initializes UART5, then periodically sends all sensor data
 * as text via LoRa transparent mode.
 */
void lora_thread_entry(void *parameter)
{
    char text_buf[LORA_TEXT_BUF_SIZE];

    uart5_init();

    LOG_I("LoRa thread started (UART5, 115200 baud).");

    rt_thread_mdelay(3000);

    while (1)
    {
        lora_build_text(text_buf, sizeof(text_buf));
        uart5_send(text_buf);

        LOG_I("Sent text payload, ppm=%u, flow=%d.%03d",
              (unsigned int)g_methane_ppm,
              (int)Flow,
              abs_int((int)((Flow - (int)Flow) * 1000)));

        rt_thread_mdelay(LORA_SEND_INTERVAL);
    }
}
