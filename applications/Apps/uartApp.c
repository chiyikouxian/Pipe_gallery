/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-18     Administrator       the first version
 */
#include "uartApp.h"

#define UART4_DEVICE_NAME "uart4" //使用 UART4
#define UART5_DEVICE_NAME "uart5" //使用 UART5

// 创建一个缓冲区来存储接收到的数据
char uart4_buffer;
char uart5_buffer;
//存储接收的数据
char uart4_recived_data[UART4_RECIVED_DATA_SIZE];       // 用来存储接收到的数据
int uart4_recived_data_index = 0;                // 用来记录接收数据的索引
char uart5_recived_data[UART5_RECIVED_DATA_SIZE];       // 用来存储接收到的数据
int uart5_recived_data_index = 0;                // 用来记录接收数据的索引

//创建device
rt_device_t uart4_device;
rt_device_t uart5_device;
//配置文件
struct serial_configure uart4_config = RT_SERIAL_CONFIG_DEFAULT;
struct serial_configure uart5_config = RT_SERIAL_CONFIG_DEFAULT;
//配置信号量
struct rt_semaphore uart4_sem;
struct rt_semaphore uart5_sem;

//回调函数
rt_err_t uart4_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&uart4_sem);
    return RT_EOK;
}

rt_err_t uart5_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&uart5_sem);
    return RT_EOK;
}

void uart4_init()
{
    uart4_device = rt_device_find(UART4_DEVICE_NAME);  //查找UART4设备
    if (uart4_device == RT_NULL)
    {
        rt_kprintf("未找到 UART设备！\n");
        return;
    }

    uart4_config.baud_rate = 115200;    //修改波特率
    uart4_config.bufsz = 256;         //修改缓冲区
    //应用配置
    rt_device_control(uart4_device, RT_DEVICE_CTRL_CONFIG, &uart4_config);
    //打开串口
//    rt_device_open(uart4_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
    rt_device_open(uart4_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(uart4_device, uart4_rx_ind);
    rt_sem_init(&uart4_sem, "rx_uart4_sem", 0, RT_IPC_FLAG_FIFO);
}

void uart5_init()
{
    uart5_device = rt_device_find(UART5_DEVICE_NAME);  //查找UART5设备
    if (uart5_device == RT_NULL)
    {
        rt_kprintf("未找到 UART设备！\n");
        return;
    }

    uart5_config.baud_rate = 9600;    //修改波特率
    uart5_config.bufsz = 256;         //修改缓冲区
    //应用配置
    rt_device_control(uart5_device, RT_DEVICE_CTRL_CONFIG, &uart5_config);
    //打开串口
    rt_device_open(uart5_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(uart5_device, uart5_rx_ind);
    rt_sem_init(&uart5_sem, "rx_uart5_sem", 0, RT_IPC_FLAG_FIFO);
}

// 向 UART 发送数据
void uart4_send(const char *data)
{
    // 发送数据
    rt_device_write(uart4_device, 0, data, strlen(data));
}

void uart5_send(const char *data)
{
    // 发送数据
    rt_device_write(uart5_device, 0, data, strlen(data));
}

// 创建一个线程来执行串口操作
void uart4_thread_entry(void *parameter)
{
    uart4_init();
    uart4_send("hello uart4!\n");
    while(1)
    {
        // 从 UART 设备中读取数据
        while (rt_device_read(uart4_device, 0, &uart4_buffer, 1) != 1)
        {
            rt_sem_take(&uart4_sem, RT_WAITING_FOREVER);
        }
        // 如果接收到数据，将其存入 buffer 中
        if (uart4_recived_data_index < UART4_RECIVED_DATA_SIZE - 1)  // 保证不溢出
        {
            uart4_recived_data[uart4_recived_data_index++] = uart4_buffer;  // 将字符存入uart4_recived_data
        }
    }
}

void uart5_thread_entry(void *parameter)
{
    uart5_init();
    uart5_send("hello uart5!\n");
    while(1)
    {
        // 从 UART 设备中读取数据
        while (rt_device_read(uart5_device, 0, &uart5_buffer, 1) != 1)
        {
            rt_sem_take(&uart5_sem, RT_WAITING_FOREVER);
        }
        // 如果接收到数据，将其存入 buffer 中
        if (uart5_recived_data_index < UART5_RECIVED_DATA_SIZE - 1)  // 保证不溢出
        {
            uart5_recived_data[uart5_recived_data_index++] = uart5_buffer;  // 将字符存入uart4_recived_data
        }
        rt_kprintf("uart5_recived_data:%s\n", uart5_recived_data);
    }
}

void uart4_buffer_clear()
{
    // 清空 buffer 和索引，为下一次接收准备
    memset(uart4_recived_data, 0x00, sizeof(uart4_recived_data));
    uart4_recived_data_index = 0;
}

void uart5_buffer_clear()
{
    // 清空 buffer 和索引，为下一次接收准备
    memset(uart5_recived_data, 0x00, sizeof(uart5_recived_data));
    uart5_recived_data_index = 0;
}

//// 主函数，创建串口操作线程
//int main(void)
//{
//    // 创建一个线程来进行 UART4 的发送和读取操作
//    rt_thread_t uart4_thread = rt_thread_create("uart4_thread", uart4_thread_entry, RT_NULL, 1024, 25, 10);
//    if (uart4_thread != RT_NULL)
//    {
//        rt_thread_startup(uart4_thread);
//    }
//    else
//    {
//        rt_kprintf("创建串口操作线程失败！\n");
//    }
//
//    return 0;
//}
