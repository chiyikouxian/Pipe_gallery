/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-13     RT-Thread    first version
 #include <rtdbg.h>
 * 2026-02-04     ideapad15s   add Huawei Cloud IoT
 #define DBG_TAG "main"
 * 2026-04-13     ideapad15s   add LoRa ATK-LORA-01
 */

#include "MethaneSensorApp.h"
#include "linesensor.h"
#include "huaweiCloudApp.h"
#include "loraApp.h"
#include "sht30App.h"
#include "o2SensorApp.h"
#include "ethApp.h"

#define DBG_LVL DBG_LOG

static void uart2_receive_thread_entry(void *parameter)
{
    /* call receive function in infinite wait mode */
    uart2_receive_and_print(-1);
}

/* Gas sensor: 3.3V power, USART2 */
/* Five-channel flame sensor: analog outputs A1-A5, ADC */
/* MODBUS: fixed data frame (CRC cannot be changed), USART3 as signal bus */

int main(void)
{
    /* ============================================================
     * UART assignment:
     * UART2 (PA2/PA3)    - Gas sensor (19200 baud)
     * UART3 (PB10/PB11)  - MODBUS RTU master (9600 baud, water meter + ammeter)
     * UART4 (PA11/PA12)  - ESP8266 WiFi module (115200 baud)
     * UART5 (PC12/PD2)   - ATK-LORA-01 LoRa module (115200 baud)
     * ============================================================ */

    /* UART4 receive thread - for ESP8266 communication */
    rt_thread_t uart4_thread = rt_thread_create("uart4_rx", uart4_thread_entry, RT_NULL, 1024, 25, 10);
    if (uart4_thread != RT_NULL)
        rt_thread_startup(uart4_thread);

    /* Huawei Cloud thread - report sensor data */
    huawei_cloud_init();

    /* MODBUS poll thread (UART3) */
    rt_thread_t tid1 = rt_thread_create("md_m_poll", mb_master_poll, RT_NULL, 512, MB_POLL_THREAD_PRIORITY, 10);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    /* MODBUS master send thread (UART3) */
    rt_thread_t tid2 = rt_thread_create("md_m_send", send_thread_entry, RT_NULL, 1024, MB_SEND_THREAD_PRIORITY - 2, 10);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    /* Five-channel flame sensor ADC init and read thread (A1-A5) */
    line_sensor_init();

    /* SHT30 temperature/humidity sensor (I2C1: PB6 SCL, PB7 SDA) */
    sht30_init();

    /* O2 sensor (ADC1 channel 10, PC0) */
    o2_sensor_init();

    /* Ethernet TCP client - fiber optic main link */
    if (eth_app_init() != RT_EOK)
        rt_kprintf("[ETH] Failed to initialize Ethernet TCP client!\n");
    else
        rt_kprintf("[ETH] Ethernet TCP client initialized.\n");

    /* LoRa send thread (UART5) - ATK-LORA-01 transparent mode */
    rt_thread_t lora_thread = rt_thread_create("lora_tx", lora_thread_entry, RT_NULL, 1024, 25, 10);
    if (lora_thread != RT_NULL)
        rt_thread_startup(lora_thread);

    /* UART2 receive thread - gas sensor (3.3V power) */
    rt_thread_t uart2_thread = rt_thread_create("uart2_rx",uart2_receive_thread_entry,RT_NULL,1024,25,10);
    if (uart2_thread != RT_NULL)
        rt_thread_startup(uart2_thread);

    return RT_EOK;
}
