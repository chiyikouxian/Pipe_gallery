/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-18     Administrator       the first version
 */
#ifndef APPLICATIONS_APPS_UARTAPP_H_
#define APPLICATIONS_APPS_UARTAPP_H_

#include "heads.h"

#define UART4_RECIVED_DATA_SIZE 256    //UART4接收字符串区大小
#define UART5_RECIVED_DATA_SIZE 128    //UART5接收字符串区大小

extern char uart4_recived_data[UART4_RECIVED_DATA_SIZE];// UART4用来存储接收到的数据
extern int uart4_recived_data_index;             // UART4用来记录接收数据的索引
extern char uart5_recived_data[UART5_RECIVED_DATA_SIZE];// UART5用来存储接收到的数据
extern int uart5_recived_data_index;             // UART5用来记录接收数据的索引

//回调函数
rt_err_t uart4_rx_ind(rt_device_t dev, rt_size_t size);
rt_err_t uart5_rx_ind(rt_device_t dev, rt_size_t size);
void uart4_init();
void uart5_init();
void uart4_send(const char *data);
void uart5_send(const char *data);
void uart4_thread_entry(void *parameter);
void uart5_thread_entry(void *parameter);
void uart4_buffer_clear();
void uart5_buffer_clear();

#endif /* APPLICATIONS_APPS_UARTAPP_H_ */
