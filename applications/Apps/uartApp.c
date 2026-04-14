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

#define UART4_DEVICE_NAME "uart4" /* use UART4 */
#define UART5_DEVICE_NAME "uart5" /* use UART5 */

/* one-byte temporary receive buffers */
char uart4_buffer;
char uart5_buffer;

/* receive data buffers */
char uart4_recived_data[UART4_RECIVED_DATA_SIZE];
int uart4_recived_data_index = 0;
char uart5_recived_data[UART5_RECIVED_DATA_SIZE];
int uart5_recived_data_index = 0;

/* devices */
rt_device_t uart4_device;
rt_device_t uart5_device;

/* serial configurations */
struct serial_configure uart4_config = RT_SERIAL_CONFIG_DEFAULT;
struct serial_configure uart5_config = RT_SERIAL_CONFIG_DEFAULT;

/* semaphores */
struct rt_semaphore uart4_sem;
struct rt_semaphore uart5_sem;

/* receive indication callbacks */
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
    uart4_device = rt_device_find(UART4_DEVICE_NAME);  /* find UART4 device */
    if (uart4_device == RT_NULL)
    {
        rt_kprintf("UART4 device not found!\n");
        return;
    }

    uart4_config.baud_rate = 115200;
    uart4_config.bufsz = 256;

    /* apply configuration */
    rt_device_control(uart4_device, RT_DEVICE_CTRL_CONFIG, &uart4_config);

    /* open device */
    rt_device_open(uart4_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(uart4_device, uart4_rx_ind);
    rt_sem_init(&uart4_sem, "rx_uart4_sem", 0, RT_IPC_FLAG_FIFO);
}

void uart5_init()
{
    uart5_device = rt_device_find(UART5_DEVICE_NAME);  /* find UART5 device */
    if (uart5_device == RT_NULL)
    {
        rt_kprintf("UART5 device not found!\n");
        return;
    }

    uart5_config.baud_rate = 115200;  /* LoRa ATK-LORA-01 baud rate */
    uart5_config.bufsz = 256;

    /* apply configuration */
    rt_device_control(uart5_device, RT_DEVICE_CTRL_CONFIG, &uart5_config);

    /* open device */
    rt_device_open(uart5_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(uart5_device, uart5_rx_ind);
    rt_sem_init(&uart5_sem, "rx_uart5_sem", 0, RT_IPC_FLAG_FIFO);
}

/* send text through UART4 */
void uart4_send(const char *data)
{
    rt_device_write(uart4_device, 0, data, strlen(data));
}

/* send text through UART5 */
void uart5_send(const char *data)
{
    rt_device_write(uart5_device, 0, data, strlen(data));
}

/* send binary data through UART5 */
void uart5_send_bytes(const rt_uint8_t *data, rt_size_t len)
{
    rt_device_write(uart5_device, 0, data, len);
}

/* UART4 receive thread */
void uart4_thread_entry(void *parameter)
{
    uart4_init();
    uart4_send("hello uart4!\n");
    while(1)
    {
        /* read one byte from UART device */
        while (rt_device_read(uart4_device, 0, &uart4_buffer, 1) != 1)
        {
            rt_sem_take(&uart4_sem, RT_WAITING_FOREVER);
        }

        /* store received byte into buffer */
        if (uart4_recived_data_index < UART4_RECIVED_DATA_SIZE - 1)
        {
            uart4_recived_data[uart4_recived_data_index++] = uart4_buffer;
        }
    }
}

/* UART5 receive thread */
void uart5_thread_entry(void *parameter)
{
    uart5_init();
    uart5_send("hello uart5!\n");
    while(1)
    {
        /* read one byte from UART device */
        while (rt_device_read(uart5_device, 0, &uart5_buffer, 1) != 1)
        {
            rt_sem_take(&uart5_sem, RT_WAITING_FOREVER);
        }

        /* store received byte into buffer */
        if (uart5_recived_data_index < UART5_RECIVED_DATA_SIZE - 1)
        {
            uart5_recived_data[uart5_recived_data_index++] = uart5_buffer;
        }
        rt_kprintf("uart5_recived_data:%s\n", uart5_recived_data);
    }
}

void uart4_buffer_clear()
{
    /* clear buffer before next receive operation */
    memset(uart4_recived_data, 0x00, sizeof(uart4_recived_data));
    uart4_recived_data_index = 0;
}

void uart5_buffer_clear()
{
    /* clear buffer before next receive operation */
    memset(uart5_recived_data, 0x00, sizeof(uart5_recived_data));
    uart5_recived_data_index = 0;
}
