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

#define UART2_DEVICE_NAME    "uart2" /* UART2_TX---PA2   UART2_RX---PA3 */
#define PACKET_SIZE          19      /* fixed 19-byte packet */

/* global methane sensor data, for cloud report */
rt_uint16_t g_methane_ppm = 0;
rt_uint8_t  g_methane_lel = 0;

/* receive packet buffer */
static rt_uint8_t uart2_received_data[PACKET_SIZE];

/**
 * @brief Validate and parse one complete methane sensor frame
 *
 * @param buffer_in input packet buffer, fixed 19 bytes
 */
static void Response_FrameCheck_Uart(rt_uint8_t *buffer_in)
{
    rt_uint8_t data_len = buffer_in[2];          /* 0x13 = 19 bytes */
    rt_uint8_t checksum = 0;
    rt_uint16_t methane = 0;
    rt_uint8_t lel = 0;
    rt_uint8_t alarm = 0;

    /* checksum verification */
    for (rt_uint8_t i = 0; i < data_len - 1; i++)
    {
        checksum += buffer_in[i];
    }

    /* validate checksum and frame marker */
    if (checksum == buffer_in[data_len - 1] && buffer_in[3] == 0xAA)
    {
        /* extract PPM value directly */
        /* byte order: buffer_in[11] is high byte, buffer_in[10] is low byte */
        methane = ((rt_uint16_t)buffer_in[11] << 8) | (rt_uint16_t)buffer_in[10];
        lel = buffer_in[12];
        alarm = buffer_in[13];

        /* update global variables for cloud report */
        g_methane_ppm = methane;
        g_methane_lel = lel;

        /* print parsed result */
        rt_kprintf("gas:%d.%d ppm, levle: %d%%, alarm:0x%02X\n", methane/1000,methane%1000, lel, alarm);
    }
    else
    {
        /* validation failed, print error info */
        rt_kprintf("Validation failed: checksum=%02X (Expectation=%02X), buffer[3]=%02X\n",
                   checksum, buffer_in[data_len - 1], buffer_in[3]);
    }
}

/**
 * @brief Receive UART2 data and print in hexadecimal format
 *
 * @param timeout receive timeout in ms, -1 means wait forever
 * @return rt_err_t RT_EOK on success, negative value on failure
 */
rt_err_t uart2_receive_and_print(rt_int32_t timeout)
{
    rt_device_t serial;
    rt_uint8_t data;
    rt_size_t result;
    rt_uint8_t last_byte = 0;  /* previous byte, used to detect AC AC frame header */
    rt_uint8_t in_packet = 0;  /* whether currently inside a packet */
    rt_uint8_t byte_count = 0; /* current packet byte count */
    rt_uint8_t pending_ac = 0; /* pending AC byte, used to detect AC AC frame header */
    rt_uint8_t data_index = 0; /* packet buffer write index */

    /* find UART device */
    serial = rt_device_find(UART2_DEVICE_NAME);
    if (serial == RT_NULL)
    {
        rt_kprintf("Cannot find %s device!\n", UART2_DEVICE_NAME);
        return -RT_ERROR;
    }

    /* open device in read/write mode */
    if (rt_device_open(serial, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("Failed to open %s device!\n", UART2_DEVICE_NAME);
        return -RT_ERROR;
    }

    /* configure UART baud rate to 19200 */
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

    /* receive loop */
    while (1)
    {
        /* read one byte from UART */
        result = rt_device_read(serial, 0, &data, 1);

        if (result > 0)
        {
            /* process pending AC byte first */
            if (pending_ac != 0)
            {
                /* if current byte is also AC, then AC AC frame header is detected */
                if (data == 0xAC)
                {
                    /* if there is previous data, start a new line */
                    if (byte_count > 0)
                    {
                        rt_kprintf("\r\n");
                        byte_count = 0;
                        data_index = 0;
                    }
                    /* AC AC starts a new packet */
                    rt_kprintf("%02X %02X ", pending_ac, data);
                    byte_count = 2;
                    in_packet = 1;
                    uart2_received_data[0] = pending_ac;
                    uart2_received_data[1] = data;
                    data_index = 2;
                }
                else
                {
                    /* not AC AC, output pending AC and current byte */
                    rt_kprintf("%02X %02X ", pending_ac, data);
                    byte_count += 2;
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
                /* detect AC byte, save it first and wait for next byte */
                if (data == 0xAC)
                {
                    pending_ac = 0xAC;
                }
                else
                {
                    /* normal byte, print directly */
                    rt_kprintf("%02X ", data);
                    byte_count++;
                    if (data_index < PACKET_SIZE)
                    {
                        uart2_received_data[data_index++] = data;
                    }
                }
            }

            /* once 19 bytes are received, parse one full packet */
            if (byte_count >= PACKET_SIZE)
            {
                rt_kprintf("\r\n");
                Response_FrameCheck_Uart(uart2_received_data);
                byte_count = 0;
                data_index = 0;
                in_packet = 0;
            }

            last_byte = data;
        }
        else if (timeout > 0)
        {
            /* timeout mode: wait a little and decrease timeout */
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
            /* timeout is zero, exit directly */
            break;
        }
        else
        {
            /* timeout < 0 means wait forever */
            rt_thread_mdelay(10);
        }
    }

    /* close device */
    rt_device_close(serial);

    return RT_EOK;
}
