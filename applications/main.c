/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-13     RT-Thread    first version
 */

#include "MethaneSensorApp.h"
#include "linesensor.h"
#include <rtdbg.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG

static void uart2_receive_thread_entry(void *parameter)
{
    /* 在线程中调用接收函数，设置永久等待模式 */
    uart2_receive_and_print(-1);
}

/* 可燃气体传感器3.3V供电,使用USART2 */
/* 循迹传感器5V供电 */
/* MODBUS使用固定请求报文(尤其是CRC校验码不能改变)，使用USART3作为信号线进行轮循 */

int main(void)
{
    /* ============================================================
     * 串口分配说明：
     * UART2 (PA2/PA3)  - 可燃气体传感器 (19200波特率)
     * UART3 (PB10/PB11)  - MODBUS RTU 主站 (9600波特率, 水表+电表)
     * UART4 (PC10/PC11)  - ESP8266 WiFi模块 (115200波特率, 已禁用)
     * ============================================================ */

    // MODBUS轮询线程 (使用UART3)
    rt_thread_t tid1 = rt_thread_create("md_m_poll", mb_master_poll, RT_NULL, 512, MB_POLL_THREAD_PRIORITY, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
        rt_kprintf("Failed to create MODBUS Looping thread!\n");
    }

    // MODBUS主站线程 (使用UART3)
    rt_thread_t tid2 = rt_thread_create("md_m_send", send_thread_entry, RT_NULL, 1024, MB_SEND_THREAD_PRIORITY - 2, 10);
    if (tid2 != RT_NULL)
    {
        rt_thread_startup(tid2);
    }
    else
    {
        rt_kprintf("Failed to create MODBUS main site thread!\n");
    }

    /* 循环检测器ADC初始化与读取线程 */
    /* 5V供电 */
    if (line_sensor_init() != RT_EOK)
    {
        rt_kprintf("Failed to initialize line sensor ADC!\n");
    }
    else
    {
        rt_kprintf("Line sensor ADC initialized successfully.\n");
    }

    // 创建一个线程来控制 UART4 的发送和读取数据 (用于ESP8266模块)
//    rt_thread_t uart4_thread = rt_thread_create("uart4_thread", uart4_thread_entry, RT_NULL, 1024, 25, 10);
//    if (uart4_thread != RT_NULL)
//    {
//        rt_thread_startup(uart4_thread);
//    }
//    else
//    {
//        rt_kprintf("创建串口4线程失败！\n");
//        goto __exit;
//    }


//    // 创建一个线程来控制网络
//    rt_thread_t wifi_thread = rt_thread_create("wifi_thread", wifi_thread_entry, RT_NULL, 2048, 25, 10);
//    if (wifi_thread != RT_NULL)
//    {
//        rt_thread_startup(wifi_thread);
//        uart_test();
//    }
//    else
//    {
//        rt_kprintf("创建网络处理线程失败！\n");
//        goto __exit;
//    }

    /* 创建串口2接收线程 - 可燃气体传感器线程 */
    /* 3.3V供电 */
    rt_thread_t uart2_thread = rt_thread_create("uart2_rx",uart2_receive_thread_entry,RT_NULL,1024,25,10);

    if (uart2_thread != RT_NULL)
    {
        rt_thread_startup(uart2_thread);
        rt_kprintf("UART2 receive thread started.\n");
    }
    else
    {
        rt_kprintf("Failed to create UART2 receive thread!\n");
        /* 如果创建线程失败，直接调用函数来接收数据 */
        uart2_receive_and_print(-1);
    }

    return RT_EOK;
}
