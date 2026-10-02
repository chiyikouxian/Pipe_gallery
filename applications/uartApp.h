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

#define UART4_RECIVED_DATA_SIZE 256    /* UART4 receive buffer size */
#define UART5_RECIVED_DATA_SIZE 128    /* UART5 receive buffer size */

extern char uart4_recived_data[UART4_RECIVED_DATA_SIZE];
extern int uart4_recived_data_index;
extern char uart5_recived_data[UART5_RECIVED_DATA_SIZE];
extern int uart5_recived_data_index;

/* callbacks */
rt_err_t uart4_rx_ind(rt_device_t dev, rt_size_t size);
rt_err_t uart5_rx_ind(rt_device_t dev, rt_size_t size);

/* init */
void uart4_init();
void uart5_init();

/* send */
void uart4_send(const char *data);
void uart5_send(const char *data);
void uart5_send_bytes(const rt_uint8_t *data, rt_size_t len);

/* threads */
void uart4_thread_entry(void *parameter);
void uart5_thread_entry(void *parameter);

/* buffer helpers */
void uart4_buffer_clear();
void uart5_buffer_clear();

#endif /* APPLICATIONS_APPS_UARTAPP_H_ */
