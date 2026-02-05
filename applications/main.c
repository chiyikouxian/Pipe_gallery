/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-13     RT-Thread    first version
 * 2026-02-04     ideapad15s   添加华为云上云功能
 */

#include "MethaneSensorApp.h"
#include "linesensor.h"
#include "huaweiCloudApp.h"
#include <rtdbg.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG

static void uart2_receive_thread_entry(void *parameter)
{
    /* 在此线程中调用接收函数, 无限等待模式 */
    uart2_receive_and_print(-1);
}

/* 甲烷传感器: 3.3V供电, 使用USART2 */
/* 循迹传感器: 5V供电 */
/* MODBUS使用固定数据帧(包含CRC校验码不能改变), 使用USART3作为信号线接入回路 */

int main(void)
{
    /* ============================================================
     * 串口分配说明:
     * UART2 (PA2/PA3)    - 甲烷传感器 (19200波特率)
     * UART3 (PB10/PB11)  - MODBUS RTU 主站 (9600波特率, 水表+电表)
     * UART4 (PA11/PA12)  - ESP8266 WiFi模块 (115200波特率)
     * ============================================================ */

    /* UART4接收线程 - 用于ESP8266通信 */
    rt_thread_t uart4_thread = rt_thread_create("uart4_rx", uart4_thread_entry, RT_NULL, 1024, 25, 10);
    if (uart4_thread != RT_NULL)
    {
        rt_thread_startup(uart4_thread);
        rt_kprintf("UART4 thread started for ESP8266.\n");
    }
    else
    {
        rt_kprintf("Failed to create UART4 thread!\n");
    }

    /* 华为云上云线程 - 上报ADC数据 */
    if (huawei_cloud_init() != RT_EOK)
    {
        rt_kprintf("Failed to initialize Huawei Cloud!\n");
    }
    else
    {
        rt_kprintf("Huawei Cloud initialized successfully.\n");
    }

    /* MODBUS轮询线程 (使用UART3) */
    rt_thread_t tid1 = rt_thread_create("md_m_poll", mb_master_poll, RT_NULL, 512, MB_POLL_THREAD_PRIORITY, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
        rt_kprintf("Failed to create MODBUS Looping thread!\n");
    }

    /* MODBUS主站线程 (使用UART3) */
    rt_thread_t tid2 = rt_thread_create("md_m_send", send_thread_entry, RT_NULL, 1024, MB_SEND_THREAD_PRIORITY - 2, 10);
    if (tid2 != RT_NULL)
    {
        rt_thread_startup(tid2);
    }
    else
    {
        rt_kprintf("Failed to create MODBUS main site thread!\n");
    }

    /* 循迹传感器ADC初始化及读取线程 (5V供电) */
    if (line_sensor_init() != RT_EOK)
    {
        rt_kprintf("Failed to initialize line sensor ADC!\n");
    }
    else
    {
        rt_kprintf("Line sensor ADC initialized successfully.\n");
    }

    /* 旧的WiFi代码 - 已被huaweiCloudApp.c替代 */
    /*
    // rt_thread_t uart4_thread = rt_thread_create("uart4_thread", uart4_thread_entry, RT_NULL, 1024, 25, 10);
    // rt_thread_t wifi_thread = rt_thread_create("wifi_thread", wifi_thread_entry, RT_NULL, 2048, 25, 10);
    */

    /* 创建UART2接收线程 - 甲烷传感器线程 (3.3V供电) */
    rt_thread_t uart2_thread = rt_thread_create("uart2_rx",uart2_receive_thread_entry,RT_NULL,1024,25,10);

    if (uart2_thread != RT_NULL)
    {
        rt_thread_startup(uart2_thread);
        rt_kprintf("UART2 receive thread started.\n");
    }
    else
    {
        rt_kprintf("Failed to create UART2 receive thread!\n");
        /* 如果创建线程失败, 直接调用函数在主线程运行 */
        uart2_receive_and_print(-1);
    }

    return RT_EOK;
}
