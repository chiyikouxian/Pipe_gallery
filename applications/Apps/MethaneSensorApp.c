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
#define PACKET_SIZE          19  /* ÿ�����ݰ��̶�19���ֽ� */

/* global methane sensor data, for cloud report */
rt_uint16_t g_methane_ppm = 0;
rt_uint8_t  g_methane_lel = 0;

/* ���ݰ������� */
static rt_uint8_t uart2_received_data[PACKET_SIZE];

/**
 * @brief ����֡У��ͽ�������
 *
 * @param buffer_in ���ݰ���������19���ֽڣ�
 */
static void Response_FrameCheck_Uart(rt_uint8_t *buffer_in)
{
    rt_uint8_t data_len = buffer_in[2];          /* 0x13 = 19 bytes */
    rt_uint8_t checksum = 0;
    rt_uint16_t methane = 0;
    rt_uint8_t lel = 0;
    rt_uint8_t alarm = 0;

    /* У�����֤ */
    for (rt_uint8_t i = 0; i < data_len - 1; i++)
    {
        checksum += buffer_in[i];
    }

    /* ��֤У��ͺ����ݸ�ʽ */
    if (checksum == buffer_in[data_len - 1] && buffer_in[3] == 0xAA)
    {
        /* ֱ����ȡPPMֵ(���赥λת��) */
        /* ע�⣺�ֽ�˳���� buffer_in[11]�����ֽڣ�<< 8 | buffer_in[10]�����ֽڣ� */
        methane = ((rt_uint16_t)buffer_in[11] << 8) | (rt_uint16_t)buffer_in[10];
        lel = buffer_in[12];
        alarm = buffer_in[13];

        /* update global variables for cloud report */
        g_methane_ppm = methane;
        g_methane_lel = lel;

        /* ��ӡ������� */
        rt_kprintf("gas:%d.%d ppm, levle: %d%%, alarm:0x%02X\n", methane/1000,methane%1000, lel, alarm);
    }
    else
    {
        /* У��ʧ��,��ӡ������Ϣ */
        rt_kprintf("Validation failed: checksum=%02X (Expectation=%02X), buffer[3]=%02X\n",
                   checksum, buffer_in[data_len - 1], buffer_in[3]);
    }
}

/**
 * @brief �Ӵ���2�������ݲ���ʮ�����Ƹ�ʽ��ӡ������̨
 *
 * @param timeout ���ճ�ʱʱ�䣨���룩��-1��ʾ���õȴ�
 * @return rt_err_t ����RT_EOK��ʾ�ɹ�������ֵ��ʾʧ��
 */
rt_err_t uart2_receive_and_print(rt_int32_t timeout)
{
    rt_device_t serial;
    rt_uint8_t data;
    rt_size_t result;
    rt_uint8_t last_byte = 0;  /* ��һ���ֽڣ����ڼ�� AC AC ֡ͷ */
    rt_uint8_t in_packet = 0;  /* �Ƿ������ݰ��� */
    rt_uint8_t byte_count = 0; /* ��ǰ�е��ֽڼ��� */
    rt_uint8_t pending_ac = 0; /* �������AC�ֽڣ����ڴ���AC AC֡ͷ�� */
    rt_uint8_t data_index = 0; /* ���ݰ����������� */

    /* ���Ҵ����豸 */
    serial = rt_device_find(UART2_DEVICE_NAME);
    if (serial == RT_NULL)
    {
        rt_kprintf("Cannot find %s device!\n", UART2_DEVICE_NAME);
        return -RT_ERROR;
    }

    /* �Զ�д��ʽ���豸 */
    if (rt_device_open(serial, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("Failed to open %s device!\n", UART2_DEVICE_NAME);
        return -RT_ERROR;
    }

    /* ���ô��ڲ�����Ϊ19200 */
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

    /* ѭ���������� */
    while (1)
    {
        /* �Ӵ��ڶ�ȡһ���ֽ� */
        result = rt_device_read(serial, 0, &data, 1);

        if (result > 0)
        {
            /* ����д������AC�ֽڣ��ȴ��� */
            if (pending_ac != 0)
            {
                /* �����ǰ�ֽ�Ҳ��AC��˵����AC AC֡ͷ */
                if (data == 0xAC)
                {
                    /* �����ǰ���Ѿ������ݣ��Ȼ��� */
                    if (byte_count > 0)
                    {
                        rt_kprintf("\r\n");
                        byte_count = 0;
                        data_index = 0;  /* ���û�������������ʼ���� */
                    }
                    /* �������AC��Ϊ���еĿ�ʼ */
                    rt_kprintf("%02X %02X ", pending_ac, data);
                    byte_count = 2;
                    in_packet = 1;
                    /* ���浽������ */
                    uart2_received_data[0] = pending_ac;
                    uart2_received_data[1] = data;
                    data_index = 2;
                }
                else
                {
                    /* ����AC AC������������AC�͵�ǰ�ֽ� */
                    rt_kprintf("%02X %02X ", pending_ac, data);
                    byte_count += 2;
                    /* ���浽������ */
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
                /* ��⵽AC�ֽڣ��ȱ��治������ȴ���һ���ֽ� */
                if (data == 0xAC)
                {
                    pending_ac = 0xAC;
                }
                else
                {
                    /* ��ͨ�ֽڣ�ֱ����� */
                    rt_kprintf("%02X ", data);
                    byte_count++;
                    /* ���浽������ */
                    if (data_index < PACKET_SIZE)
                    {
                        uart2_received_data[data_index++] = data;
                    }
                }
            }

            /* ����Ƿ�ﵽ19���ֽڣ���������в����� */
            if (byte_count >= PACKET_SIZE)
            {
                rt_kprintf("\r\n");
                /* ��������֡У��ͽ������� */
                Response_FrameCheck_Uart(uart2_received_data);
                byte_count = 0;
                data_index = 0;
                in_packet = 0;
            }

            /* ������һ���ֽ� */
            last_byte = data;
        }
        else if (timeout > 0)
        {
            /* ��������˳�ʱ���ȴ�һ��ʱ����˳� */
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
            /* ��ʱʱ��Ϊ0�������˳� */
            break;
        }
        else
        {
            /* timeout < 0�����õȴ�������ѭ�� */
            rt_thread_mdelay(10);
        }
    }

    /* �ر��豸 */
    rt_device_close(serial);

    return RT_EOK;
}
