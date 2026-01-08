/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-17     ideapad15s       the first version
 */
#include "heads.h"
#include <drivers/serial.h>
#include "MethaneSensorApp.h"

#define UART2_DEVICE_NAME    "uart2"//UART2_TX---PA2   UART2_RX---PA3
#define PACKET_SIZE          19  /* 每个数据包固定19个字节 */

/* 数据包缓冲区 */
static rt_uint8_t uart2_received_data[PACKET_SIZE];

/**
 * @brief 数据帧校验和解析函数
 *
 * @param buffer_in 数据包缓冲区（19个字节）
 */
static void Response_FrameCheck_Uart(rt_uint8_t *buffer_in)
{
    rt_uint8_t data_len = buffer_in[2];          /* 0x13 = 19 bytes */
    rt_uint8_t checksum = 0;
    rt_uint16_t methane = 0;
    rt_uint8_t lel = 0;
    rt_uint8_t alarm = 0;

    /* 校验和验证 */
    for (rt_uint8_t i = 0; i < data_len - 1; i++)
    {
        checksum += buffer_in[i];
    }

    /* 验证校验和和数据格式 */
    if (checksum == buffer_in[data_len - 1] && buffer_in[3] == 0xAA)
    {
        /* 直接提取PPM值(无需单位转换) */
        /* 注意：字节顺序是 buffer_in[11]（高字节）<< 8 | buffer_in[10]（低字节） */
        methane = ((rt_uint16_t)buffer_in[11] << 8) | (rt_uint16_t)buffer_in[10];
        lel = buffer_in[12];
        alarm = buffer_in[13];

        /* 打印解析结果 */
        rt_kprintf("gas:%d.%d ppm, levle: %d%%\n", methane/1000,methane%1000, lel);
    }
    else
    {
        /* 校验失败,打印错误信息 */
        rt_kprintf("Validation failed: checksum=%02X (Expectation=%02X), buffer[3]=%02X\n",
                   checksum, buffer_in[data_len - 1], buffer_in[3]);
    }
}

/**
 * @brief 从串口2接收数据并以十六进制格式打印到控制台
 *
 * @param timeout 接收超时时间（毫秒），-1表示永久等待
 * @return rt_err_t 返回RT_EOK表示成功，其他值表示失败
 */
rt_err_t uart2_receive_and_print(rt_int32_t timeout)
{
    rt_device_t serial;
    rt_uint8_t data;
    rt_size_t result;
    rt_uint8_t last_byte = 0;  /* 上一个字节，用于检测 AC AC 帧头 */
    rt_uint8_t in_packet = 0;  /* 是否在数据包中 */
    rt_uint8_t byte_count = 0; /* 当前行的字节计数 */
    rt_uint8_t pending_ac = 0; /* 待输出的AC字节（用于处理AC AC帧头） */
    rt_uint8_t data_index = 0; /* 数据包缓冲区索引 */

    /* 查找串口设备 */
    serial = rt_device_find(UART2_DEVICE_NAME);
    if (serial == RT_NULL)
    {
        rt_kprintf("Cannot find %s device!\n", UART2_DEVICE_NAME);
        return -RT_ERROR;
    }

    /* 以读写方式打开设备 */
    if (rt_device_open(serial, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("Failed to open %s device!\n", UART2_DEVICE_NAME);
        return -RT_ERROR;
    }

    /* 配置串口波特率为19200 */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_19200;
    if (rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config) != RT_EOK)
    {
        rt_kprintf("Failed to configure UART2 baud rate!\n");
        rt_device_close(serial);
        return -RT_ERROR;
    }

    rt_kprintf("UART2 receive started (19200 baud), waiting for data...\n");
    rt_kprintf("Received HEX data:\n");

    /* 循环接收数据 */
    while (1)
    {
        /* 从串口读取一个字节 */
        result = rt_device_read(serial, 0, &data, 1);

        if (result > 0)
        {
            /* 如果有待输出的AC字节，先处理 */
            if (pending_ac != 0)
            {
                /* 如果当前字节也是AC，说明是AC AC帧头 */
                if (data == 0xAC)
                {
                    /* 如果当前行已经有数据，先换行 */
                    if (byte_count > 0)
                    {
                        rt_kprintf("\r\n");
                        byte_count = 0;
                        data_index = 0;  /* 重置缓冲区索引，开始新行 */
                    }
                    /* 输出两个AC作为新行的开始 */
                    rt_kprintf("%02X %02X ", pending_ac, data);
                    byte_count = 2;
                    in_packet = 1;
                    /* 保存到缓冲区 */
                    uart2_received_data[0] = pending_ac;
                    uart2_received_data[1] = data;
                    data_index = 2;
                }
                else
                {
                    /* 不是AC AC，输出待输出的AC和当前字节 */
                    rt_kprintf("%02X %02X ", pending_ac, data);
                    byte_count += 2;
                    /* 保存到缓冲区 */
                    if (data_index < PACKET_SIZE)
                    {
                        uart2_received_data[data_index++] = pending_ac;
                    }
                    if (data_index < PACKET_SIZE)
                    {
                        uart2_received_data[data_index++] = data;
                    }
                }
                pending_ac = 0;
            }
            else
            {
                /* 检测到AC字节，先保存不输出，等待下一个字节 */
                if (data == 0xAC)
                {
                    pending_ac = 0xAC;
                }
                else
                {
                    /* 普通字节，直接输出 */
                    rt_kprintf("%02X ", data);
                    byte_count++;
                    /* 保存到缓冲区 */
                    if (data_index < PACKET_SIZE)
                    {
                        uart2_received_data[data_index++] = data;
                    }
                }
            }

            /* 检查是否达到19个字节，如果是则换行并解密 */
            if (byte_count >= PACKET_SIZE)
            {
                rt_kprintf("\r\n");
                /* 调用数据帧校验和解析函数 */
                Response_FrameCheck_Uart(uart2_received_data);
                byte_count = 0;
                data_index = 0;
                in_packet = 0;
            }

            /* 更新上一个字节 */
            last_byte = data;
        }
        else if (timeout > 0)
        {
            /* 如果设置了超时，等待一段时间后退出 */
            rt_thread_mdelay(10);
            timeout -= 10;
            if (timeout <= 0)
            {
                rt_kprintf("\nUART2 receive timeout!\n");
                break;
            }
        }
        else if (timeout == 0)
        {
            /* 超时时间为0，立即退出 */
            break;
        }
        else
        {
            /* timeout < 0，永久等待，继续循环 */
            rt_thread_mdelay(10);
        }
    }

    /* 关闭设备 */
    rt_device_close(serial);

    return RT_EOK;
}
